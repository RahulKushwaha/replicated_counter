//
// Created by Rahul  Kushwaha on 12/29/22.
//

#ifndef LOGSTORAGE_INMEMORYMETADATASTORE_H
#define LOGSTORAGE_INMEMORYMETADATASTORE_H

#include "../include/MetadataStore.h"

#include <mutex>
#include <map>

namespace rk::projects::durable_log {

class InMemoryMetadataStore: public MetadataStore {
 public:
  explicit InMemoryMetadataStore();

  std::optional<MetadataConfig> getConfig(VersionId versionId) override;
  std::optional<MetadataConfig> getConfigUsingLogId(LogId logId) override;

  VersionId getCurrentVersionId() override;

  void
  compareAndAppendRange(VersionId versionId,
                        MetadataConfig newMetadataConfig) override;

  ~InMemoryMetadataStore() override = default;

 private:
  struct MetadataConfigComparator {
    bool
    operator()(const MetadataConfig &left, const MetadataConfig &right) const {
      return left.version_id() < right.version_id();
    }
  };

  struct State {
    std::mutex mtx;
    std::map<VersionId, MetadataConfig> configs_;
    std::map<LogId, MetadataConfig> logIdToConfig_;
  };

  std::unique_ptr<State> state_;
};

}
#endif //LOGSTORAGE_INMEMORYMETADATASTORE_H
