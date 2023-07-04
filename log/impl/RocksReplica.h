//
// Created by Rahul  Kushwaha on 6/28/23.
//
#pragma once
#include "log/include/Replica.h"
#include "log/utils/OrderedCompletionQueue.h"
#include "persistence/KVStoreLite.h"

namespace rk::projects::durable_log {

class RocksReplica : public Replica {
public:
  explicit RocksReplica(std::string id, std::string name,
                        std::shared_ptr<MetadataStore> metadataStore,
                        std::shared_ptr<persistence::KVStoreLite> kvStore);
  std::string getId() override;
  std::string getName() override;
  coro<folly::Unit> append(std::optional<LogId> globalCommitIndex,
                           VersionId versionId, LogId logId,
                           std::string logEntryPayload, bool skipSeal) override;
  coro<std::variant<LogEntry, LogReadError>> getLogEntry(VersionId versionId,
                                                         LogId logId) override;

  LogId getCommitIndex(VersionId versionId) override;
  LogId seal(VersionId versionId) override;

  ~RocksReplica() override = default;

private:
  struct KeyFormat {
    std::string logKey() { return "VERSION_ID|{}|LOG_ID|{}"; }
    std::string logKeyPrefix() { return "VERSION_ID|{}|LOG_ID|"; }
    std::string sealKey() { return "VERSION_ID|{}|SEAL|"; }
    std::string startIndexKey() { return "VERSION_ID|{}|START_INDEX|"; }
    std::string endIndexKey() { return "VERSION_ID|{}|END_INDEX|"; }
  };

  KeyFormat formatter;

private:
  std::string id_;
  std::string name_;
  std::shared_ptr<MetadataStore> metadataStore_;
  std::shared_ptr<persistence::KVStoreLite> kvStore_;
  std::map<VersionId, std::unique_ptr<utils::OrderedCompletionQueue<LogId>>>
      completionQueueLookup_;
};

} // namespace rk::projects::durable_log
