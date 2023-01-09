//
// Created by Rahul  Kushwaha on 12/29/22.
//

#include "InMemoryMetadataStore.h"

namespace rk::project::counter {

InMemoryMetadataStore::InMemoryMetadataStore() : state_{
    std::make_unique<State>()} {}

std::optional<MetadataConfig>
InMemoryMetadataStore::getConfigUsingLogId(LogId logId) {
  std::lock_guard<std::mutex> lockGuard{state_->mtx};
  auto itr = state_->logIdToConfig_.upper_bound(logId);
  if (itr == state_->logIdToConfig_.begin()) {
    return {};
  }

  itr--;
  // Make sure logId is >= metadataConfig
  CHECK(logId >= itr->second.startindex());
  return itr->second;
}

std::optional<MetadataConfig>
InMemoryMetadataStore::getConfig(VersionId versionId) {
  std::lock_guard<std::mutex> lockGuard{state_->mtx};

  if (auto itr = state_->configs_.find(versionId);
      itr != state_->configs_.end()) {
    return itr->second;
  }

  return {};
}

VersionId InMemoryMetadataStore::getCurrentVersionId() {
  // TODO: Convert this to a ReaderWriter lock.
  std::lock_guard<std::mutex> lockGuard{state_->mtx};
  if (state_->configs_.empty()) {
    return 0;
  }

  return state_->configs_.rbegin()->first;
}

void InMemoryMetadataStore::compareAndAppendRange(VersionId versionId,
                                                  MetadataConfig newMetadataConfig) {
  std::lock_guard<std::mutex> lockGuard{state_->mtx};

  if (state_->configs_.empty()) {
    if (versionId != 0) {
      throw OptimisticConcurrencyException{};
    }

    state_->configs_[newMetadataConfig.versionid()] = newMetadataConfig;
    state_->logIdToConfig_[newMetadataConfig.startindex()] = newMetadataConfig;
    return;
  }

  auto lastConfig = state_->configs_.rbegin();
  if (lastConfig->first == versionId
      && lastConfig->first + 1 == newMetadataConfig.versionid()) {
    state_->configs_[newMetadataConfig.versionid()] = newMetadataConfig;
    state_->logIdToConfig_[newMetadataConfig.startindex()] = newMetadataConfig;
    return;
  }

  throw OptimisticConcurrencyException{};
}

}