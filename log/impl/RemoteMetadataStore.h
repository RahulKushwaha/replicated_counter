//
// Created by Rahul  Kushwaha on 3/26/23.
//

#pragma once

#include "log/include/MetadataStore.h"
#include "log/client/MetadataStoreClient.h"

namespace rk::projects::durable_log {

class RemoteMetadataStore: public MetadataStore {
 public:
  explicit RemoteMetadataStore(std::shared_ptr<client::MetadataStoreClient> metadataStoreClient)
      : client_{std::move(metadataStoreClient)} {}

  std::optional<MetadataConfig> getConfig(VersionId versionId) override {
    return client_->getConfig(versionId).get();
  }

  std::optional<MetadataConfig> getConfigUsingLogId(LogId logId) override {
    return client_->getConfigUsingLogId(logId).get();
  }

  VersionId getCurrentVersionId() override {
    return client_->getCurrentVersionId().get();
  }

  void compareAndAppendRange(VersionId versionId,
                             MetadataConfig newMetadataConfig) override {
    client_->compareAndAppendRange(versionId, newMetadataConfig);
  }

  void printConfigChain() override {
    client_->printConfigChain().semi().get();
  }

 private:
  std::shared_ptr<client::MetadataStoreClient> client_;
};

}
