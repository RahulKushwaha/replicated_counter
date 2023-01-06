//
// Created by Rahul  Kushwaha on 12/29/22.
//

#include "InMemoryMetadataStore.h"

namespace rk::project::counter {

InMemoryMetadataStore::InMemoryMetadataStore() : state_{
    std::make_unique<State>()} {}

MetadataConfig InMemoryMetadataStore::getConfigUsingLogId(LogId logId) {
  std::lock_guard<std::mutex> lockGuard{state_->mtx};
  auto itr = state_->logIdToConfig_.upper_bound(logId);
  if (itr == state_->logIdToConfig_.begin()) {
    return MetadataConfig{};
  }

  itr--;
  // Make sure logId is >= metadataConfig
  CHECK(logId >= itr->second.startindex());
  return itr->second;
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

  if (state_->configs_.empty()) {
    state_->configs_[versionId] = newMetadataConfig;
    state_->logIdToConfig_[newMetadataConfig.startindex()] = newMetadataConfig;
    return;
  }

  auto lastConfig = state_->configs_.rbegin();
  if (lastConfig->first == versionId
      && lastConfig->first + 1 == newMetadataConfig.versionid()) {
    state_->configs_[versionId] = newMetadataConfig;
    state_->logIdToConfig_[newMetadataConfig.startindex()] = newMetadataConfig;
  }

  throw OptimisticConcurrencyException{};
}

}