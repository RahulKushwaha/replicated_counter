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
  if (itr == state_->logIdToConfig_.end()) {
    itr--;
  }

  while (true) {
    if (logId >= itr->second.startindex() && logId < itr->second.endindex()) {
      return itr->second;
    }

    if (itr == state_->logIdToConfig_.begin()) {
      break;
    }

    itr--;
  }

  LOG(ERROR) << "Metadata Block for log_id " << logId << " not present";

  return {};
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
    // Now we have an implicit contract that the new index is 1 greater than
    // last config index.
    state_->configs_[versionId].set_endindex(newMetadataConfig.previousversionendindex());
    state_->configs_[newMetadataConfig.versionid()] = newMetadataConfig;
    state_->logIdToConfig_[newMetadataConfig.startindex()] = newMetadataConfig;
    return;
  }

  throw OptimisticConcurrencyException{};
}

}