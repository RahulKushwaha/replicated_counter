//
// Created by Rahul  Kushwaha on 1/2/23.
//

#include "ReplicaImpl.h"
#include "VectorBasedNanoLog.h"
namespace rk::project::counter {

ReplicaImpl::ReplicaImpl(
    std::string id,
    std::string name,
    std::shared_ptr<NanoLogStore> nanoLogStore,
    std::shared_ptr<MetadataStore> metadataStore)
    : id_{std::move(id)}, name_{std::move(name)},
      nanoLogStore_{std::move(nanoLogStore)},
      metadataStore_{std::move(metadataStore)} {
}

std::string ReplicaImpl::getId() {
  return id_;
}

std::string ReplicaImpl::getName() {
  return name_;
}

folly::SemiFuture<folly::Unit>
ReplicaImpl::append(LogId logId,
                    std::string logEntryPayload) {
  MetadataConfig config = metadataStore_->getConfigUsingLogId(logId);
  std::shared_ptr<NanoLog>
      nanoLog = nanoLogStore_->getNanoLog(config.versionid());

  // If there is nanolog for the versionId, we need to create one.
  if (!nanoLog) {
    nanoLog = std::make_shared<VectorBasedNanoLog>
        ("id",
         "name",
         std::to_string(config.versionid()),
         config.startindex(),
         std::numeric_limits<std::int64_t>::max(),
         false);

    nanoLogStore_->add(config.versionid(), nanoLog);
  }

  return nanoLog->append(logId, logEntryPayload)
      .deferValue([](auto &&) {
      });
}

folly::SemiFuture<std::variant<LogEntry, LogReadError>>
ReplicaImpl::getLogEntry(LogId logId) {
  MetadataConfig config = metadataStore_->getConfigUsingLogId(logId);
  std::shared_ptr<NanoLog>
      nanoLog = nanoLogStore_->getNanoLog(config.versionid());

  return folly::makeSemiFuture(nanoLog->getLogEntry(logId));
}

LogId ReplicaImpl::seal() {
  return 0;
}

}
