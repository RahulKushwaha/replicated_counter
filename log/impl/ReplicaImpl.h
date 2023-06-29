//
// Created by Rahul  Kushwaha on 1/2/23.
//

#pragma once

#include "log/include/NanoLogStore.h"
#include "log/include/Replica.h"
#include "log/include/Sequencer.h"
#include "log/utils/OrderedCompletionQueue.h"

namespace rk::projects::durable_log {

class ReplicaImpl : public Replica {
public:
  explicit ReplicaImpl(std::string id, std::string name,
                       std::shared_ptr<NanoLogStore> nanoLogStore,
                       std::shared_ptr<MetadataStore> metadataStore,
                       bool local = false);

  std::string getId() override;
  std::string getName() override;

  coro<folly::Unit> append(std::optional<LogId> globalCommitIndex,
                           VersionId versionId, LogId logId,
                           std::string logEntryPayload,
                           bool skipSeal = false) override;

  coro<std::variant<LogEntry, LogReadError>> getLogEntry(VersionId versionId,
                                                         LogId logId) override;

  LogId getLocalCommitIndex(VersionId versionId) override;

  LogId seal(VersionId versionId) override;

  ~ReplicaImpl() override = default;

private:
  std::string id_;
  std::string name_;
  std::shared_ptr<NanoLogStore> nanoLogStore_;
  std::shared_ptr<MetadataStore> metadataStore_;
  const bool local_;
  std::shared_ptr<std::mutex> mtx_;
};

} // namespace rk::projects::durable_log
