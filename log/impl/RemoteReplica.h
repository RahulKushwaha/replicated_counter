//
// Created by Rahul  Kushwaha on 2/26/23.
//

#pragma once

#include "log/client/ReplicaClient.h"
#include "log/include/Replica.h"

namespace rk::projects::durable_log {

class RemoteReplica final : public Replica {
public:
  explicit RemoteReplica(std::shared_ptr<client::ReplicaClient> replicaClient)
      : replicaClient_{std::move(replicaClient)} {}

  std::string getId() override { return replicaClient_->getId().get(); }

  std::string getName() override { throw NotImplementedException{}; }

  folly::SemiFuture<folly::Unit> append(std::optional<LogId> globalCommitIndex,
                                        VersionId versionId, LogId logId,
                                        std::string logEntryPayload,
                                        bool skipSeal) override {
    return replicaClient_->append(globalCommitIndex, versionId, logId,
                                  std::move(logEntryPayload), skipSeal);
  }

  folly::SemiFuture<std::variant<LogEntry, LogReadError>>
  getLogEntry(VersionId versionId, LogId logId) override {
    return replicaClient_->getLogEntry(versionId, logId);
  }

  LogId getCommitIndex(VersionId versionId) override {
    return replicaClient_->getLocalCommitIndex(versionId).get();
  }

  LogId seal(VersionId versionId) override {
    return replicaClient_->seal(versionId).get();
  }

private:
  std::shared_ptr<client::ReplicaClient> replicaClient_;
};

} // namespace rk::projects::durable_log
