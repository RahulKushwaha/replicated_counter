//
// Created by Rahul  Kushwaha on 2/26/23.
//
#include "folly/experimental/coro/Task.h"
#include "log/include/MetadataStore.h"
#include "log/server/proto/MetadataService.grpc.pb.h"
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
  folly::coro::Task<void> printConfigChain();

private:
  std::unique_ptr<server::MetadataService::Stub> stub_;
};

} // namespace rk::projects::durable_log::client
