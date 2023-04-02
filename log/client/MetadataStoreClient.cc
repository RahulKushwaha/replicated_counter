//
// Created by Rahul  Kushwaha on 2/26/23.
//

#include "log/client/MetadataStoreClient.h"

namespace rk::projects::durable_log::client {

using namespace std::chrono_literals;
constexpr auto CLIENT_TIMEOUT = 250ms;

MetadataStoreClient::MetadataStoreClient(std::shared_ptr<grpc::Channel> channel)
    : stub_{server::MetadataService::NewStub(std::move(channel))} {}

folly::SemiFuture<MetadataConfig>
MetadataStoreClient::getConfig(VersionId versionId) {
  grpc::ClientContext context;
  context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);
  server::MetadataVersionId request;
  request.set_id(versionId);
  MetadataConfig response;

  auto status = stub_->getConfig(&context, request, &response);
  if (status.ok()) {
    return folly::makeSemiFuture(response);
  }

  return folly::makeSemiFuture<MetadataConfig>
      (folly::make_exception_wrapper<std::exception>());
}

folly::SemiFuture<MetadataConfig>
MetadataStoreClient::getConfigUsingLogId(LogId logId) {
  grpc::ClientContext context;
  context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);
  server::LogId request;
  request.set_id(logId);
  MetadataConfig response;

  auto status = stub_->getConfigUsingLogId(&context, request, &response);
  if (status.ok()) {
    return folly::makeSemiFuture(response);
  }

  return folly::makeSemiFuture<MetadataConfig>
      (folly::make_exception_wrapper<std::exception>());
}

folly::SemiFuture<VersionId> MetadataStoreClient::getCurrentVersionId() {
  grpc::ClientContext context;
  context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);
  MetadataConfig response;

  auto status =
      stub_->getCurrentConfig(&context, google::protobuf::Empty{}, &response);

  if (status.ok()) {
    return folly::makeSemiFuture<VersionId>(response.version_id());
  }

  return folly::makeSemiFuture<VersionId>
      (folly::make_exception_wrapper<std::exception>());
}

folly::SemiFuture<folly::Unit> MetadataStoreClient::compareAndAppendRange(
    VersionId versionId,
    MetadataConfig newMetadataConfig) {
  grpc::ClientContext context;
  context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);
  server::CompareAndAppendRangeRequest request;
  request.mutable_metadata_version_id()->set_id(versionId);
  request.mutable_metadata_config()->Swap(&newMetadataConfig);
  google::protobuf::Empty response;

  auto status = stub_->compareAndAppendRange(&context, request, &response);

  if (status.ok()) {
    return folly::makeSemiFuture();
  }

  return folly::makeSemiFuture<folly::Unit>
      (folly::make_exception_wrapper<std::exception>());
}

}