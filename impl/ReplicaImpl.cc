//
// Created by Rahul  Kushwaha on 1/2/23.
//

#include "ReplicaImpl.h"
namespace rk::project::counter {

ReplicaImpl::ReplicaImpl(
    std::shared_ptr<NanoLogStore> nanoLogStore,
    std::shared_ptr<MetadataStore> metadataStore)
    : nanoLogStore_{std::move(nanoLogStore)},
      metadataStore_{std::move(metadataStore)} {
}

std::string ReplicaImpl::getId() {
  return std::string();
}

std::string ReplicaImpl::getName() {
  return std::string();
}

folly::SemiFuture<folly::Unit>
ReplicaImpl::append(LogId logId,
                    std::string logEntryPayload) {
  MetadataConfig config = metadataStore_->getConfigUsingLogId(logId);
  std::shared_ptr<NanoLog>
      nanoLog = nanoLogStore_->getNanoLog(config.versionid());

  return nanoLog->append(logId, logEntryPayload)
      .deferValue([](auto &&) {
      });
}

LogId ReplicaImpl::seal() {
  return 0;
}

}
