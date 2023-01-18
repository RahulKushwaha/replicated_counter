//
// Created by Rahul  Kushwaha on 12/30/22.
//
#include "VirtualLogImpl.h"
#include "../utils/FutureUtils.h"
#include <folly/executors/InlineExecutor.h>

#include <random>

namespace rk::project::counter {

VirtualLogImpl::VirtualLogImpl(
    std::string id,
    std::string name,
    std::shared_ptr<Sequencer> sequencer,
    std::vector<std::shared_ptr<Replica>> replicaSet,
    std::shared_ptr<MetadataStore> metadataStore)
    :
    id_{std::move(id)},
    name_{std::move(name)},
    sequencer_{std::move(sequencer)},
    replicaSet_{std::move(replicaSet)},
    metadataStore_{std::move(metadataStore)} {
}

std::string VirtualLogImpl::getId() {
  return id_;
}

std::string VirtualLogImpl::getName() {
  return name_;
}

folly::SemiFuture<LogId> VirtualLogImpl::append(std::string logEntryPayload) {
  return sequencer_->append(logEntryPayload)
      .via(&folly::InlineExecutor::instance());
}

std::variant<LogEntry, LogReadError> VirtualLogImpl::getLogEntry(LogId logId) {
  return {LogReadError::NotFound};
}

void VirtualLogImpl::reconfigure() {
  VersionId versionId = metadataStore_->getCurrentVersionId();

  // Make a copy of the replicaSet
  std::vector<std::shared_ptr<Replica>> replicaSet = replicaSet_;

  std::random_device randomDevice;
  std::mt19937 g(randomDevice());
  std::shuffle(replicaSet.begin(), replicaSet.end(), g);

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
    for (auto &replica: replicaSet_) {
      folly::SemiFuture<folly::Unit>
          appendFuture =
          replica->append(logEntry.logId, logEntry.payload, true)
              .via(&folly::InlineExecutor::instance());

      appendFutures.emplace_back(std::move(appendFuture));
    }

    utils::anyNSuccessful(std::move(appendFutures), replicaSet_.size() / 2 + 1)
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
    newConfig.set_previousversionid(versionId);
    newConfig.set_previousversionendindex(maxLogId);

    newConfig.set_startindex(maxLogId + 1);
    newConfig.set_endindex(std::numeric_limits<std::int64_t>::max());
    newConfig.set_versionid(versionId + 1);

    metadataStore_->compareAndAppendRange(versionId, newConfig);
  }
}

}