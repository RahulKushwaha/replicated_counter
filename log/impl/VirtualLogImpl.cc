//
// Created by Rahul  Kushwaha on 12/30/22.
//
#include "VirtualLogImpl.h"
#include "../utils/FutureUtils.h"
#include <folly/executors/InlineExecutor.h>
#include <folly/futures/Retrying.h>

#include <random>

namespace rk::projects::durable_log {

namespace {

MetadataConfig
getConfigOrFatalFailure(const std::shared_ptr<MetadataStore> &metadataStore,
                        VersionId metadataConfigVersionId) {
  auto &&config = metadataStore->getConfig(metadataConfigVersionId);
  CHECK(config.has_value());

  return config.value();
}

}

VirtualLogImpl::VirtualLogImpl(
    std::string id,
    std::string name,
    std::shared_ptr<Sequencer> sequencer,
    std::vector<std::shared_ptr<Replica>> replicaSet,
    std::shared_ptr<MetadataStore> metadataStore,
    VersionId metadataConfigVersionId)
    : id_{std::move(id)},
      name_{std::move(name)},
      metadataStore_{std::move(metadataStore)},
      state_{std::make_unique<State>(
          State{
              getConfigOrFatalFailure(metadataStore_, metadataConfigVersionId),
              std::move(sequencer),
              std::move(replicaSet)}
      )} {}

std::string VirtualLogImpl::getId() {
  return id_;
}

std::string VirtualLogImpl::getName() {
  return name_;
}

folly::SemiFuture<LogId> VirtualLogImpl::sync() {
  return state_->sequencer->latestAppendPosition();
}

folly::SemiFuture<LogId> VirtualLogImpl::append(std::string logEntryPayload) {
  return state_->sequencer->append(logEntryPayload)
      .via(&folly::InlineExecutor::instance());
}

folly::SemiFuture<std::variant<LogEntry, LogReadError>>
VirtualLogImpl::getLogEntry(LogId logId) {
  return folly::futures::retrying(
      [maxRetries = state_->replicaSet.size()](
          std::size_t count,
          const folly::exception_wrapper &) {
        if (count < maxRetries) {
          return folly::makeSemiFuture(true);
        } else {
          return folly::makeSemiFuture(false);
        }
      }, [this, logId](std::size_t retryAttempt) {
        auto &replica = state_->replicaSet[retryAttempt];
        return replica->getLogEntry(logId)
            .via(&folly::InlineExecutor::instance())
            .then([logId, retryAttempt](folly::Try<std::variant<LogEntry,
                                                                LogReadError>>
                                        &&logEntryResult) {
              if (logEntryResult.hasException()) {
                LOG(ERROR) << logEntryResult.exception().what();
                LOG(INFO) << "Failed to find the logId: [" << logId
                          << "] at replica. Attempt: [" << retryAttempt << "]";
                folly::throw_exception(logEntryResult.exception());
              }

              auto &&variantResult = logEntryResult.value();
              if (std::holds_alternative<LogEntry>(variantResult)) {
                LOG(INFO) << "Successfully found the logId: [" << logId
                          << "] at replica. Attempt: [" << retryAttempt << "]";
                return variantResult;
              }

              LOG(INFO) << "Failed to find the logId: [" << logId
                        << "] at replica. Attempt: [" << retryAttempt << "]";
              throw std::exception{};
            });
      });
}

void VirtualLogImpl::reconfigure() {
  VersionId versionId = metadataStore_->getCurrentVersionId();

  // Make a copy of the replicaSet
  std::vector<std::shared_ptr<Replica>> replicaSet = state_->replicaSet;

  std::random_device randomDevice;
  std::mt19937 twisterEngine(randomDevice());
  std::shuffle(replicaSet.begin(), replicaSet.end(), twisterEngine);

  auto numberOfReplicas = replicaSet.size();
  auto majorityCount = numberOfReplicas / 2 + 1;
  replicaSet.resize(majorityCount);

  LogId minLogId = HighestNonExistingLogId, maxLogId = LowestNonExistingLogId;
  for (auto &replica: replicaSet) {
    auto endLogId = replica->seal(versionId);

    minLogId = std::min(minLogId, endLogId);
    maxLogId = std::max(maxLogId, endLogId);
  }

  LOG(INFO) << "MinLogID: " << minLogId << " MaxLogId: " << maxLogId;
  // Re-replicate all the entries from minLogId to MaxLogId
  for (LogId logId = minLogId; logId < maxLogId; logId++) {
    std::vector<folly::SemiFuture<LogEntry>> futures;
    for (auto &replica: replicaSet) {
      auto future = replica->getLogEntry(logId)
          .via(&folly::InlineExecutor::instance())
          .then([](folly::Try<std::variant<LogEntry, LogReadError>> &&result) {
            if (result.hasValue()
                && std::holds_alternative<LogEntry>(result.value())) {
              return std::get<LogEntry>(result.value());
            }

            throw std::exception{};
          });

      futures.emplace_back(std::move(future));
    }

    LogEntry logEntry =
        folly::collectAnyWithoutException(futures.begin(), futures.end())
            .via(&folly::InlineExecutor::instance())
            .thenValue([](std::pair<std::size_t, LogEntry> &&result) {
              LOG(INFO) << "Successfully Fetched the LogEntry: "
                        << "ReplicaId: " << result.first << " LogId: "
                        << result.second.logId << " Payload: "
                        << result.second.payload;
              return result.second;
            })
            .get();

    std::vector<folly::SemiFuture<folly::Unit>> appendFutures;
    for (auto &replica: state_->replicaSet) {
      folly::SemiFuture<folly::Unit>
          appendFuture =
          replica->append(logEntry.logId, logEntry.payload, true)
              .via(&folly::InlineExecutor::instance());

      appendFutures.emplace_back(std::move(appendFuture));
    }

    utils::anyNSuccessful(std::move(appendFutures),
                          state_->replicaSet.size() / 2 + 1)
        .via(&folly::InlineExecutor::instance())
        .thenValue([logId](auto &&) {
          return logId;
        })
        .get();
  }

  // Quorum of the nanologs have been re-replicated.
  // Update the MetadataConfig block.
  auto metadataConfig = metadataStore_->getConfig(versionId);

  if (metadataConfig) {
    MetadataConfig newConfig{};
    newConfig.set_previous_version_id(versionId);
    newConfig.set_previous_version_end_index(maxLogId);

    newConfig.set_start_index(maxLogId);
    newConfig.set_end_index(std::numeric_limits<std::int64_t>::max());
    newConfig.set_version_id(versionId + 1);

    // Set the sequencer id;
    newConfig.mutable_sequencer_config()->set_id(state_->sequencer->getId());

    metadataStore_->compareAndAppendRange(versionId, newConfig);
  }
}

}