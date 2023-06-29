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

  LogId getLocalCommitIndex(VersionId versionId) override;
  LogId seal(VersionId versionId) override;

  ~RocksReplica() override = default;

private:
  inline std::string getLogKey(VersionId versionId, LogId logId);
  inline std::string getLogKeyPrefix(VersionId versionId);
  inline std::string getSealKey(VersionId versionId);
  inline std::string getCommitIndexKey(VersionId versionId);

  coro<std::optional<LogId>> getLatestLogId(VersionId versionId);

private:
  std::string id_;
  std::string name_;
  std::shared_ptr<MetadataStore> metadataStore_;
  std::shared_ptr<persistence::KVStoreLite> kvStore_;
  std::map<VersionId, std::unique_ptr<utils::OrderedCompletionQueue<LogId>>>
      completionQueueLookup_;
};

} // namespace rk::projects::durable_log
