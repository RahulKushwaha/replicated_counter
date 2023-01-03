//
// Created by Rahul  Kushwaha on 12/29/22.
//

#include "InMemoryMetadataStore.h"

namespace rk::project::counter {

InMemoryMetadataStore::InMemoryMetadataStore() : state_{
    std::make_unique<State>()} {}

MetadataConfig InMemoryMetadataStore::getConfigUsingLogId(LogId logId) {
  return MetadataConfig{};
}

MetadataConfig InMemoryMetadataStore::getConfig(VersionId versionId) {
  std::lock_guard<std::mutex> lockGuard{state_->mtx};

  if (auto itr = state_->configs_.find(versionId);
      itr != state_->configs_.end()) {
    return itr->second;
  }

  MetadataConfig config{};
  config.set_versionid(std::numeric_limits<std::int64_t>::min());

  return config;
}

void InMemoryMetadataStore::compareAndAppendRange(VersionId versionId,
                                                  MetadataConfig newMetadataConfig) {
  std::lock_guard<std::mutex> lockGuard{state_->mtx};

  auto lastConfig = state_->configs_.rbegin();
  if (lastConfig->first == versionId
      && lastConfig->first + 1 == newMetadataConfig.versionid()) {
    state_->configs_[versionId] = newMetadataConfig;
  }

  throw OptimisticConcurrencyException{};
}

}