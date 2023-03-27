//
// Created by Rahul  Kushwaha on 2/26/23.
//

#pragma once

#include "log/include/Replica.h"
#include "log/client/ReplicaClient.h"

namespace rk::projects::durable_log {

class RemoteReplica final: public Replica {
 public:
  explicit RemoteReplica(std::shared_ptr<client::ReplicaClient> replicaClient) :
      replicaClient_{std::move(replicaClient)} {}

  std::string getId() override {
    return replicaClient_->getId().get();
  }

  std::string getName() override {
    throw NotImplementedException{};
  }

  folly::SemiFuture<folly::Unit>
  append(LogId logId, std::string logEntryPayload, bool skipSeal) override {
    LOG(INFO) << "ID: " << getId() << " Append Request: [LogID: " << logId
              << ", Payload: "
              << logEntryPayload << "]";

    return replicaClient_->append(logId, std::move(logEntryPayload));
  }

  folly::SemiFuture<std::variant<LogEntry, LogReadError>>
  getLogEntry(LogId logId) override {
    return replicaClient_->getLogEntry(logId);
  }

  LogId getLocalCommitIndex() override {
    return replicaClient_->getLocalCommitIndex().get();
  }

  LogId seal(VersionId versionId) override {
    return replicaClient_->seal(versionId).get();
  }

 private:
  std::shared_ptr<client::ReplicaClient> replicaClient_;
};

}

