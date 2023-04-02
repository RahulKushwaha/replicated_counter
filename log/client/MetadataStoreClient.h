//
// Created by Rahul  Kushwaha on 2/26/23.
//
#include "log/server/proto/MetadataService.grpc.pb.h"
#include "log/include/MetadataStore.h"
#include "folly/experimental/coro/Task.h"
#include <folly/futures/Future.h>
#include <glog/logging.h>
#include <grpc++/grpc++.h>

namespace rk::projects::durable_log::client {

class MetadataStoreClient {
 public:
  explicit MetadataStoreClient(std::shared_ptr<grpc::Channel> channel);

  folly::SemiFuture<MetadataConfig> getConfig(VersionId versionId);
  folly::SemiFuture<MetadataConfig> getConfigUsingLogId(LogId logId);
  folly::SemiFuture<VersionId> getCurrentVersionId();
  folly::SemiFuture<folly::Unit>
  compareAndAppendRange(VersionId versionId, MetadataConfig newMetadataConfig);

 private:
  std::unique_ptr<server::MetadataService::Stub> stub_;
};

}
