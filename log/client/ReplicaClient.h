//
// Created by Rahul  Kushwaha on 2/26/23.
//
#include "../include/Common.h"
#include "../include/MetadataStore.h"
#include "log/server/proto/Replica.grpc.pb.h"

#include <folly/futures/Future.h>
#include <glog/logging.h>
#include <grpc++/grpc++.h>

namespace rk::projects::durable_log::client {

class ReplicaClient {
 public:
  explicit ReplicaClient(std::shared_ptr<grpc::Channel> channel);

  folly::SemiFuture<std::string> getId();
  folly::SemiFuture<folly::Unit> append(std::optional<LogId> globalCommitIndex,
                                        VersionId versionId, LogId logId,
                                        std::string logEntryPayload,
                                        bool skipSeal);
  folly::SemiFuture<std::variant<LogEntry, LogReadError>> getLogEntry(
      VersionId versionId, LogId logId);
  folly::SemiFuture<LogId> getLocalCommitIndex(VersionId versionId);
  folly::SemiFuture<LogId> seal(VersionId versionId);
  coro<LogId> trim(VersionId versionId, LogId logId);

 private:
  std::unique_ptr<server::ReplicaService::Stub> stub_;
};

}  // namespace rk::projects::durable_log::client
