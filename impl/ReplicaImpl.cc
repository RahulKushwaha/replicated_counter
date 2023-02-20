//
// Created by Rahul  Kushwaha on 1/2/23.
//

#include "ReplicaImpl.h"
#include "VectorBasedNanoLog.h"
#include <folly/executors/InlineExecutor.h>

namespace rk::project::counter {

ReplicaImpl::ReplicaImpl(
    std::string id,
    std::string name,
    std::shared_ptr<NanoLogStore> nanoLogStore,
    std::shared_ptr<MetadataStore> metadataStore,
    bool local)
    : id_{std::move(id)}, name_{std::move(name)},
      nanoLogStore_{std::move(nanoLogStore)},
      metadataStore_{std::move(metadataStore)},
      local_{local} {
}

std::string ReplicaImpl::getId() {
  return id_;
}

std::string ReplicaImpl::getName() {
  return name_;
}

folly::SemiFuture<folly::Unit>
ReplicaImpl::append(LogId logId,
                    std::string logEntryPayload, bool skipSeal) {
  std::optional<MetadataConfig>
      config = metadataStore_->getConfigUsingLogId(logId);

  if (!config.has_value()) {
    return folly::makeSemiFuture<folly::Unit>(folly::make_exception_wrapper<
        MetadataBlockNotPresent>());
  }

  std::shared_ptr<NanoLog>
      nanoLog = nanoLogStore_->getNanoLog(config->versionid());

  // If there is nanolog for the versionId, we need to create one.
  if (!nanoLog) {
    nanoLog = std::make_shared<VectorBasedNanoLog>
        ("id",
         "name",
         std::to_string(config->versionid()),
         config->startindex(),
         std::numeric_limits<std::int64_t>::max(),
         false);

    nanoLogStore_->add(config->versionid(), nanoLog);
  }

  return nanoLog->append(logId, logEntryPayload, skipSeal)
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
ReplicaImpl::getLogEntry(LogId logId) {
  std::optional<MetadataConfig>
      config = metadataStore_->getConfigUsingLogId(logId);

  if (!config.has_value()) {
    return folly::makeSemiFuture<std::variant<LogEntry, LogReadError>>
        (folly::make_exception_wrapper<MetadataBlockNotPresent>());
  }

  std::shared_ptr<NanoLog>
      nanoLog = nanoLogStore_->getNanoLog(config->versionid());

  if (!nanoLog) {
    return folly::makeSemiFuture<std::variant<LogEntry, LogReadError>>
        (std::variant<LogEntry, LogReadError>{LogReadError::NotFound});
  }

  return folly::makeSemiFuture(nanoLog->getLogEntry(logId));
}

LogId ReplicaImpl::getLocalCommitIndex() {
  VersionId versionId = metadataStore_->getCurrentVersionId();
  auto config = metadataStore_->getConfigUsingLogId(versionId);
  if (!config.has_value()) {
    throw MetadataBlockNotPresent{};
  }

  auto nanoLog = nanoLogStore_->getNanoLog(versionId);
  return nanoLog->getLocalCommitIndex();
}

LogId ReplicaImpl::seal(VersionId versionId) {
  std::optional<MetadataConfig> config = metadataStore_->getConfig(versionId);
  if (!config.has_value()) {
    throw MetadataBlockNotPresent{};
  }

  std::shared_ptr<NanoLog> nanoLog = nanoLogStore_->getNanoLog(versionId);
  if (nanoLog) {
    return nanoLog->seal();
  }

  throw NanoLogLogNotAvailable{};
}

}
