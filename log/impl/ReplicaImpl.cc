//
// Created by Rahul  Kushwaha on 1/2/23.
//

#include "ReplicaImpl.h"
#include "InMemoryNanoLog.h"
#include "folly/executors/InlineExecutor.h"
#include "log/utils/UuidGenerator.h"

namespace rk::projects::durable_log {

ReplicaImpl::ReplicaImpl(std::string id, std::string name,
                         std::shared_ptr<NanoLogStore> nanoLogStore,
                         std::shared_ptr<MetadataStore> metadataStore,
                         std::shared_ptr<NanoLogFactory> nanoLogFactory,
                         NanoLogType nanoLogType)
    : id_{std::move(id)}, name_{std::move(name)},
      nanoLogStore_{std::move(nanoLogStore)},
      metadataStore_{std::move(metadataStore)},
      mtx_{std::make_shared<std::mutex>()},
      nanoLogFactory_{std::move(nanoLogFactory)}, nanoLogType_{nanoLogType} {}

std::string ReplicaImpl::getId() { return id_; }

std::string ReplicaImpl::getName() { return name_; }

coro<folly::Unit> ReplicaImpl::append(std::optional<LogId> globalCommitIndex,
                                      VersionId versionId, LogId logId,
                                      std::string logEntryPayload,
                                      bool skipSeal) {
  std::shared_ptr<NanoLog> nanoLog;

  {
    std::lock_guard lk{*mtx_};

    nanoLog = nanoLogStore_->getNanoLog(versionId);
    // If there is nanolog for the versionId, we need to create one.
    if (!nanoLog) {
      auto config = co_await metadataStore_->getConfig(versionId);
      if (!config.has_value()) {
        throw MetadataBlockNotFound{};
      }

      nanoLog = co_await nanoLogFactory_->makeNanoLog(
          nanoLogType_, utils::UuidGenerator::instance().generate(),
          "VectorBasedNanoLog", std::to_string(versionId),
          config->start_index(), std::numeric_limits<std::int64_t>::max(),
          false);

      nanoLogStore_->add(versionId, nanoLog);
    }
  }

  co_await nanoLog->append(globalCommitIndex, logId, logEntryPayload, skipSeal);

  co_return folly::unit;
}

coro<std::variant<LogEntry, LogReadError>>
ReplicaImpl::getLogEntry(VersionId versionId, LogId logId) {
  std::shared_ptr<NanoLog> nanoLog;
  {
    std::lock_guard lk{*mtx_};

    nanoLog = nanoLogStore_->getNanoLog(versionId);
    if (!nanoLog) {
      co_return std::variant<LogEntry, LogReadError>{LogReadError::NotFound};
    }
  }

  co_return co_await nanoLog->getLogEntry(logId);
}

coro<LogId> ReplicaImpl::getCommitIndex(VersionId versionId) {
  std::lock_guard lk{*mtx_};

  auto nanoLog = nanoLogStore_->getNanoLog(versionId);
  co_return co_await nanoLog->getLocalCommitIndex();
}

coro<LogId> ReplicaImpl::seal(VersionId versionId) {
  std::lock_guard lk{*mtx_};

  std::shared_ptr<NanoLog> nanoLog = nanoLogStore_->getNanoLog(versionId);

  auto config = co_await metadataStore_->getConfig(versionId);
  if (!config.has_value()) {
    throw MetadataBlockNotFound{};
  }

  // There might be a case where an empty segment might be sealed
  // due to multiple seal commands one after the another.
  if (!nanoLog) {
    nanoLog = co_await nanoLogFactory_->makeNanoLog(
        nanoLogType_, utils::UuidGenerator::instance().generate(),
        "VectorBasedNanoLog", std::to_string(versionId), config->start_index(),
        std::numeric_limits<std::int64_t>::max(), false);

    nanoLogStore_->add(versionId, nanoLog);

    nanoLog = nanoLogStore_->getNanoLog(versionId);
  }

  if (nanoLog) {
    co_return co_await nanoLog->seal();
  }

  throw NanoLogLogNotAvailable{};
}

coro<LogId> ReplicaImpl::trim(VersionId versionId, LogId logId) {
  std::shared_ptr<NanoLog> nanoLog;
  {
    std::lock_guard lk{*mtx_};
    nanoLog = nanoLogStore_->getNanoLog(versionId);
  }

  if (nanoLog) {
    co_return co_await nanoLog->trim(logId);
  }

  throw std::runtime_error{"nanolog not present"};
}

} // namespace rk::projects::durable_log
