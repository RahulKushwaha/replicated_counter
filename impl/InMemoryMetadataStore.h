//
// Created by Rahul  Kushwaha on 12/29/22.
//

#ifndef LOGSTORAGE_INMEMORYMETADATASTORE_H
#define LOGSTORAGE_INMEMORYMETADATASTORE_H

#include "../include/MetadataStore.h"

#include <mutex>
#include <map>

namespace rk::project::counter {

class InMemoryMetadataStore: public MetadataStore {
 public:
  explicit InMemoryMetadataStore();

  std::optional<MetadataConfig> getConfig(VersionId versionId) override;
  std::optional<MetadataConfig> getConfigUsingLogId(LogId logId) override;

  VersionId getCurrentVersionId() override;

  void
  compareAndAppendRange(VersionId versionId,
                        MetadataConfig newMetadataConfig) override;

 private:
  struct MetadataConfigComparator {
    bool
    operator()(const MetadataConfig &left, const MetadataConfig &right) const {
      return left.versionid() < right.versionid();
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
