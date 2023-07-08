//
// Created by Rahul  Kushwaha on 3/26/23.
//

#pragma once

#include "log/client/MetadataStoreClient.h"
#include "log/include/MetadataStore.h"

namespace rk::projects::durable_log {

class RemoteMetadataStore : public MetadataStore {
public:
  explicit RemoteMetadataStore(
      std::shared_ptr<client::MetadataStoreClient> metadataStoreClient)
      : client_{std::move(metadataStoreClient)} {}

  coro<std::optional<MetadataConfig>> getConfig(VersionId versionId) override {
    co_return client_->getConfig(versionId).get();
  }

  coro<std::optional<MetadataConfig>>
  getConfigUsingLogId(LogId logId) override {
    co_return client_->getConfigUsingLogId(logId).get();
  }

  coro<VersionId> getCurrentVersionId() override {
    co_return client_->getCurrentVersionId().get();
  }

  coro<void> compareAndAppendRange(VersionId versionId,
                                   MetadataConfig newMetadataConfig) override {
    co_await client_->compareAndAppendRange(versionId, newMetadataConfig);
  }

  void printConfigChain() override { client_->printConfigChain().semi().get(); }

private:
  std::shared_ptr<client::MetadataStoreClient> client_;
};

} // namespace rk::projects::durable_log
