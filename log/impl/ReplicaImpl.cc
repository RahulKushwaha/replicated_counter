//
// Created by Rahul  Kushwaha on 1/2/23.
//

#include "ReplicaImpl.h"
#include "VectorBasedNanoLog.h"
#include "log/utils/UuidGenerator.h"
#include "folly/executors/InlineExecutor.h"

namespace rk::projects::durable_log {

ReplicaImpl::ReplicaImpl(
    std::string id,
    std::string name,
    std::shared_ptr<NanoLogStore> nanoLogStore,
    std::shared_ptr<MetadataStore> metadataStore,
    bool local)
    : id_{std::move(id)}, name_{std::move(name)},
      nanoLogStore_{std::move(nanoLogStore)},
      metadataStore_{std::move(metadataStore)},
      local_{local},
      mtx_{std::make_shared<std::mutex>()} {
}

std::string ReplicaImpl::getId() {
  return id_;
}

std::string ReplicaImpl::getName() {
  return name_;
}

folly::SemiFuture<folly::Unit>
ReplicaImpl::append(std::optional<LogId> globalCommitIndex,
                    VersionId versionId,
                    LogId logId,
                    std::string logEntryPayload,
                    bool skipSeal) {
  std::lock_guard lk{*mtx_};

  std::shared_ptr<NanoLog>
      nanoLog = nanoLogStore_->getNanoLog(versionId);

  // If there is nanolog for the versionId, we need to create one.
  if (!nanoLog) {
    nanoLog = std::make_shared<VectorBasedNanoLog>
        (utils::UuidGenerator::instance().generate(),
         "VectorBasedNanoLog",
         std::to_string(versionId),
         logId,
         std::numeric_limits<std::int64_t>::max(),
         false);

    nanoLogStore_->add(versionId, nanoLog);
  }

  return nanoLog->append(globalCommitIndex, logId, logEntryPayload, skipSeal)
      .via(&folly::InlineExecutor::instance())
      .then([](folly::Try<LogId> &&logId) {
        if (logId.hasValue()) {
          return folly::makeSemiFuture();
        } else {
          return folly::makeSemiFuture<folly::Unit>(logId.exception());
        }
      });
}

folly::SemiFuture<std::variant<LogEntry, LogReadError>>
ReplicaImpl::getLogEntry(VersionId versionId, LogId logId) {
  std::lock_guard lk{*mtx_};

  std::shared_ptr<NanoLog>
      nanoLog = nanoLogStore_->getNanoLog(versionId);

  if (!nanoLog) {
    return folly::makeSemiFuture<std::variant<LogEntry, LogReadError>>
        (std::variant<LogEntry, LogReadError>{LogReadError::NotFound});
  }

  return folly::makeSemiFuture(nanoLog->getLogEntry(logId));
}

LogId ReplicaImpl::getLocalCommitIndex(VersionId versionId) {
  std::lock_guard lk{*mtx_};

  auto nanoLog = nanoLogStore_->getNanoLog(versionId);
  return nanoLog->getLocalCommitIndex();
}

LogId ReplicaImpl::seal(VersionId versionId) {
  std::lock_guard lk{*mtx_};

  std::shared_ptr<NanoLog> nanoLog = nanoLogStore_->getNanoLog(versionId);

  auto config = metadataStore_->getConfig(versionId);
  if (!config.has_value()) {
    throw MetadataBlockNotFound{};
  }

  // There might be a case where an empty segment might be sealed
  // due to multiple seal commands one after the another.
  if (!nanoLog) {
    nanoLog = std::make_shared<VectorBasedNanoLog>
        (utils::UuidGenerator::instance().generate(),
         "VectorBasedNanoLog",
         std::to_string(versionId),
         config->start_index(),
         std::numeric_limits<std::int64_t>::max(),
         false);

    nanoLogStore_->add(versionId, nanoLog);

    nanoLog = nanoLogStore_->getNanoLog(versionId);
  }

  if (nanoLog) {
    return nanoLog->seal();
  }

  throw NanoLogLogNotAvailable{};
}

}
