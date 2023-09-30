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

  coro<folly::Unit> append(std::optional<LogId> globalCommitIndex,
                           VersionId versionId, LogId logId,
                           std::string logEntryPayload,
                           bool skipSeal) override {
    co_return co_await replicaClient_->append(globalCommitIndex, versionId,
                                              logId, std::move(logEntryPayload),
                                              skipSeal);
  }

  coro<std::variant<LogEntry, LogReadError>> getLogEntry(VersionId versionId,
                                                         LogId logId) override {
    co_return co_await replicaClient_->getLogEntry(versionId, logId);
  }

  coro<LogId> getCommitIndex(VersionId versionId) override {
    co_return co_await replicaClient_->getLocalCommitIndex(versionId);
  }

  coro<LogId> seal(VersionId versionId) override {
    co_return co_await replicaClient_->seal(versionId);
  }

  coro<LogId> trim(VersionId versionId, LogId logId) override {
    co_return co_await replicaClient_->trim(versionId, logId);
  }

 private:
  std::shared_ptr<client::ReplicaClient> replicaClient_;
};

}  // namespace rk::projects::durable_log
