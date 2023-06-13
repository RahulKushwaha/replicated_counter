//
// Created by Rahul  Kushwaha on 12/29/22.
//

#pragma once

#include "log/include/MetadataStore.h"

#include <map>
#include <mutex>

namespace rk::projects::durable_log {

class InMemoryMetadataStore : public MetadataStore {
public:
  explicit InMemoryMetadataStore();

  std::optional<MetadataConfig> getConfig(VersionId versionId) override;
  std::optional<MetadataConfig> getConfigUsingLogId(LogId logId) override;

  VersionId getCurrentVersionId() override;

  void compareAndAppendRange(VersionId versionId,
                             MetadataConfig newMetadataConfig) override;

  void printConfigChain() override;

  ~InMemoryMetadataStore() override = default;

private:
  struct State {
    std::mutex mtx;
    std::map<VersionId, MetadataConfig> configs_;
    std::map<LogId, MetadataConfig> logIdToConfig_;
  };

  std::unique_ptr<State> state_;
};

} // namespace rk::projects::durable_log
