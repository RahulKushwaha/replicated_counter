//
// Created by Rahul  Kushwaha on 12/30/22.
//
#include "VirtualLogImpl.h"
#include "log/utils/FutureUtils.h"
#include <folly/executors/InlineExecutor.h>
#include <folly/futures/Retrying.h>

#include <random>

namespace rk::projects::durable_log {

namespace {

MetadataConfig
getConfigOrFatalFailure(const std::shared_ptr<MetadataStore> &metadataStore,
                        VersionId metadataConfigVersionId) {
  auto &&config = metadataStore->getConfig(metadataConfigVersionId);
  assert(config.has_value());

  return config.value();
}

} // namespace

VirtualLogImpl::VirtualLogImpl(std::string id, std::string name,
                               std::shared_ptr<Sequencer> sequencer,
                               std::vector<std::shared_ptr<Replica>> replicaSet,
                               std::shared_ptr<MetadataStore> metadataStore,
                               VersionId metadataConfigVersionId,
                               std::shared_ptr<Registry> registry)
    : id_{std::move(id)}, name_{std::move(name)},
      metadataStore_{std::move(metadataStore)},
      state_{std::make_unique<State>(State{
          //              getConfigOrFatalFailure(metadataStore_,
          //              metadataConfigVersionId),
          MetadataConfig{}, std::move(sequencer), std::move(replicaSet)})},
      registry_{std::move(registry)} {}

std::string VirtualLogImpl::getId() { return id_; }

std::string VirtualLogImpl::getName() { return name_; }

folly::SemiFuture<LogId> VirtualLogImpl::sync() {
  return state_->sequencer->latestAppendPosition();
}

folly::SemiFuture<LogId> VirtualLogImpl::append(std::string logEntryPayload) {
  return state_->sequencer->append(logEntryPayload)
      .via(&folly::InlineExecutor::instance());
}

folly::SemiFuture<std::variant<LogEntry, LogReadError>>
VirtualLogImpl::getLogEntry(LogId logId) {
  auto metadataConfig = metadataStore_->getConfigUsingLogId(logId);

  if (!metadataConfig.has_value()) {
    throw std::runtime_error{
        "metadataconfig does not exist for the given log id"};
  }

  return folly::futures::retrying(
      [maxRetries = state_->replicaSet.size(),
       versionId = state_->metadataConfig.version_id()](
          std::size_t count, const folly::exception_wrapper &) {
        if (count < maxRetries) {
          return folly::makeSemiFuture(true);
        } else {
          return folly::makeSemiFuture(false);
        }
      },
      [this, logId, versionId = metadataConfig.value().version_id()](
          std::size_t retryAttempt) {
        auto &replica = state_->replicaSet[retryAttempt];
        return replica->getLogEntry(versionId, logId)
            .semi()
            .via(&folly::InlineExecutor::instance())
            .then([logId, retryAttempt](
                      folly::Try<std::variant<LogEntry, LogReadError>>
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

folly::coro::Task<MetadataConfig>
VirtualLogImpl::reconfigure(MetadataConfig targetMetadataConfig) {
  VersionId versionId = metadataStore_->getCurrentVersionId();
  LOG(INFO) << "Starting Reconfiguration: VersionId: " << versionId;

  // Make a copy of the replicaSet
  std::vector<std::shared_ptr<Replica>> replicaSet = state_->replicaSet;

  std::random_device randomDevice;
  std::mt19937 twisterEngine(randomDevice());
  std::shuffle(replicaSet.begin(), replicaSet.end(), twisterEngine);

  auto numberOfReplicas = replicaSet.size();
  auto majorityCount = numberOfReplicas / 2 + 1;
  replicaSet.resize(majorityCount);

  LogId minLogId = HighestNonExistingLogId, maxLogId = LowestNonExistingLogId;
  for (auto &replica : replicaSet) {
    auto endLogId = replica->seal(versionId);

    minLogId = std::min(minLogId, endLogId);
    maxLogId = std::max(maxLogId, endLogId);
  }

  LOG(INFO) << "MinLogID: " << minLogId << " MaxLogId: " << maxLogId;
  // Re-replicate all the entries from minLogId to MaxLogId
  for (LogId logId = minLogId; logId < maxLogId; logId++) {
    std::vector<folly::SemiFuture<LogEntry>> futures;
    for (auto &replica : replicaSet) {
      auto future =
          replica->getLogEntry(versionId, logId)
              .semi()
              .via(&folly::InlineExecutor::instance())
              .then([](folly::Try<std::variant<LogEntry, LogReadError>>
                           &&result) {
                if (result.hasValue() &&
                    std::holds_alternative<LogEntry>(result.value())) {
                  return std::get<LogEntry>(result.value());
                }

                if (result.hasValue() &&
                    std::holds_alternative<LogReadError>(result.value())) {
                  std::stringstream ss;
                  ss << std::get<LogReadError>(result.value());

                  LOG(INFO) << "log read error: " << ss.str();
                  throw std::runtime_error{ss.str()};
                }

                throw result.exception().get_exception();
              });

      futures.emplace_back(std::move(future));
    }

    LogEntry logEntry =
        folly::collectAnyWithoutException(futures.begin(), futures.end())
            .via(&folly::InlineExecutor::instance())
            .thenValue([](std::pair<std::size_t, LogEntry> &&result) {
              return result.second;
            })
            .get();
    LOG(INFO) << "found log entry";
    std::vector<folly::SemiFuture<folly::Unit>> appendFutures;
    for (auto &replica : state_->replicaSet) {
      folly::SemiFuture<folly::Unit> appendFuture =
          replica->append({}, versionId, logEntry.logId, logEntry.payload, true)
              .semi()
              .via(&folly::InlineExecutor::instance());

      appendFutures.emplace_back(std::move(appendFuture));
    }

    utils::anyNSuccessful(std::move(appendFutures),
                          state_->replicaSet.size() / 2 + 1)
        .via(&folly::InlineExecutor::instance())
        .then([](folly::Try<folly::Unit> &&result) {
          if (result.hasException()) {
            if (auto *exception = result.template tryGetExceptionObject<
                                  utils::MultipleExceptions>();
                exception) {
              LOG(ERROR) << exception->getDebugString();
            }

            std::rethrow_exception(result.exception().to_exception_ptr());
          }

          return result.value();
        })
        .get();
  }

  LOG(INFO)
      << "Completed Re-Replication. All replica's have been populated to: "
      << maxLogId;

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
    auto sequencerConfig = newConfig.mutable_sequencer_config();
    sequencerConfig->CopyFrom(targetMetadataConfig.sequencer_config());

    auto replicaConfig = newConfig.mutable_replica_set_config();
    replicaConfig->CopyFrom(targetMetadataConfig.replica_set_config());

    try {
      metadataStore_->compareAndAppendRange(versionId, newConfig);
      setState(newConfig.version_id());
      state_->sequencer->start(newConfig.version_id(), newConfig.start_index());
      co_return newConfig;
    } catch (const OptimisticConcurrencyException &e) {
      LOG(INFO) << "Some other replica was able to install metadata. "
                << e.what();
    }
  }

  auto newVersionId = metadataStore_->getCurrentVersionId();

  setState(newVersionId);
  co_return *metadataStore_->getConfig(newVersionId);
}

folly::coro::Task<MetadataConfig> VirtualLogImpl::getCurrentConfig() {
  return folly::coro::makeTask(state_->metadataConfig);
}

folly::coro::Task<void> VirtualLogImpl::refreshConfiguration() {
  LOG(INFO) << "Refreshing State";
  auto versionId = metadataStore_->getCurrentVersionId();
  auto currentVersionId = state_->metadataConfig.version_id();

  CHECK(currentVersionId <= versionId);

  if (currentVersionId < versionId) {
    LOG(INFO) << "New version of metadata found: " << versionId;
    setState(versionId);
  }

  co_return;
}

void VirtualLogImpl::setState(VersionId versionId) {
  LOG(INFO) << "Installing new State";
  auto config = metadataStore_->getConfig(versionId);

  if (!config.has_value()) {
    LOG(INFO) << "Config is not present.";
  }

  LOG(INFO) << "New Sequencer: " << config->sequencer_config().id();
  std::shared_ptr<Sequencer> sequencer =
      registry_->sequencer(config->sequencer_config().id());

  CHECK(sequencer != nullptr);

  std::vector<std::shared_ptr<Replica>> replicaSet = state_->replicaSet;
  // TODO(FIX): Refresh replica information.
  //  for (const auto &replicaConfig: config->replica_set_config()) {
  //    auto replica = registry_->replica(replicaConfig.id());
  //    replicaSet.emplace_back(std::move(replica));
  //  }

  std::unique_ptr<State> state = std::make_unique<State>(
      State{*config, std::move(sequencer), std::move(replicaSet)});

  state_ = std::move(state);

  LOG(INFO) << "Completed Installing new State";
  LOG(INFO) << "Printing Metadata Chain";
  metadataStore_->printConfigChain();
}

} // namespace rk::projects::durable_log