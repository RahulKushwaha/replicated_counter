//
// Created by Rahul  Kushwaha on 2/26/23.
//

#include "log/client/MetadataStoreClient.h"

namespace rk::projects::durable_log::client {

using namespace std::chrono_literals;
constexpr auto CLIENT_TIMEOUT = 250ms;

MetadataStoreClient::MetadataStoreClient(std::shared_ptr<grpc::Channel> channel)
    : stub_{server::MetadataService::NewStub(std::move(channel))} {}

folly::SemiFuture<MetadataConfig> MetadataStoreClient::getConfig(
    VersionId versionId) {
  grpc::ClientContext context;
  context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);
  server::MetadataVersionId request;
  request.set_id(versionId);
  MetadataConfig response;

  auto status = stub_->getConfig(&context, request, &response);
  if (status.ok()) {
    return folly::makeSemiFuture(response);
  }

  auto err = folly::make_exception_wrapper<std::runtime_error>(
      context.debug_error_string());
  return folly::makeSemiFuture<MetadataConfig>(std::move(err));
}

folly::SemiFuture<MetadataConfig> MetadataStoreClient::getConfigUsingLogId(
    LogId logId) {
  grpc::ClientContext context;
  context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);
  server::LogId request;
  request.set_id(logId);
  MetadataConfig response;

  auto status = stub_->getConfigUsingLogId(&context, request, &response);
  if (status.ok()) {
    return folly::makeSemiFuture(response);
  }

  auto err = folly::make_exception_wrapper<std::runtime_error>(
      context.debug_error_string());
  return folly::makeSemiFuture<MetadataConfig>(std::move(err));
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

  auto err = folly::make_exception_wrapper<std::runtime_error>(
      context.debug_error_string());
  return folly::makeSemiFuture<VersionId>(std::move(err));
}

folly::SemiFuture<folly::Unit> MetadataStoreClient::compareAndAppendRange(
    MetadataConfig newMetadataConfig) {
  grpc::ClientContext context;
  context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);
  server::CompareAndAppendRangeRequest request;
  request.mutable_metadata_config()->Swap(&newMetadataConfig);
  google::protobuf::Empty response;

  auto status = stub_->compareAndAppendRange(&context, request, &response);

  if (status.ok()) {
    return folly::makeSemiFuture();
  }

  return folly::makeSemiFuture<folly::Unit>(
      folly::make_exception_wrapper<std::runtime_error>(
          "debug string: " + context.debug_error_string()));
}

folly::coro::Task<void> MetadataStoreClient::printConfigChain() {
  grpc::ClientContext context;
  context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);
  google::protobuf::Empty request;
  google::protobuf::Empty response;

  auto status = stub_->printConfigChain(&context, request, &response);

  if (status.ok()) {
    co_return;
  }

  throw folly::make_exception_wrapper<std::runtime_error>(
      "debug string: " + context.debug_error_string());
}

}  // namespace rk::projects::durable_log::client