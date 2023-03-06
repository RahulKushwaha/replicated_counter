//
// Created by Rahul  Kushwaha on 2/26/23.
//
#include "../include/Common.h"

#include "log/server/proto/Replica.grpc.pb.h"
#include "../include/MetadataStore.h"
#include <folly/futures/Future.h>
#include <glog/logging.h>
#include <grpc++/grpc++.h>

namespace rk::projects::durable_log::client {

class ReplicaClient {
 public:
  explicit ReplicaClient(std::shared_ptr<grpc::Channel> channel);

  folly::SemiFuture<std::string> getId();
  folly::SemiFuture<folly::Unit> append(LogId logId,
                                        std::string logEntryPayload);
  folly::SemiFuture<std::variant<LogEntry, LogReadError>>
  getLogEntry(LogId logId);
  folly::SemiFuture<LogId> getLocalCommitIndex();
  folly::SemiFuture<LogId> seal(VersionId versionId);

 private:
  std::unique_ptr<server::ReplicaService::Stub> stub_;
};

}
