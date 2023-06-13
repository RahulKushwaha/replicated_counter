//
// Created by Rahul  Kushwaha on 2/26/23.
//

#include "SequencerClient.h"

namespace rk::projects::durable_log::client {

SequencerClient::SequencerClient(std::shared_ptr<grpc::Channel> channel)
    : stub_{server::SequencerService::NewStub(std::move(channel))} {}

folly::SemiFuture<std::string> SequencerClient::getId() {
  grpc::ClientContext context;
  google::protobuf::Empty request;
  server::IdResponse response;

  grpc::Status status = stub_->getId(&context, request, &response);

  if (status.ok()) {
    return folly::makeSemiFuture(response.id());
  }

  auto err = folly::make_exception_wrapper<std::runtime_error>(
      context.debug_error_string());
  return folly::makeSemiFuture<std::string>(std::move(err));
}

folly::SemiFuture<LogId> SequencerClient::append(std::string logEntryPayload) {
  grpc::ClientContext context;
  server::LogIdResponse response;
  server::SequencerAppendRequest request;
  request.set_payload(std::move(logEntryPayload));

  auto status = stub_->append(&context, request, &response);
  if (status.ok()) {
    return folly::makeSemiFuture<LogId>(response.log_id());
  }

  auto err = folly::make_exception_wrapper<std::runtime_error>(
      context.debug_error_string());
  return folly::makeSemiFuture<LogId>(std::move(err));
}

folly::SemiFuture<LogId> SequencerClient::latestAppendPosition() {
  grpc::ClientContext context;
  google::protobuf::Empty request;
  server::LogIdResponse response;

  auto status = stub_->latestAppendPosition(&context, request, &response);
  if (status.ok()) {
    return folly::makeSemiFuture<LogId>(response.log_id());
  }

  auto err = folly::make_exception_wrapper<std::runtime_error>(
      context.debug_error_string());
  return folly::makeSemiFuture<LogId>(std::move(err));
}

folly::SemiFuture<bool> SequencerClient::isAlive() {
  grpc::ClientContext context;
  google::protobuf::Empty request;
  server::IsAliveResponse response;

  auto status = stub_->isAlive(&context, request, &response);

  if (status.ok()) {
    return folly::makeSemiFuture<bool>(true);
  }

  auto err = folly::make_exception_wrapper<std::runtime_error>(
      context.debug_error_string());
  return folly::makeSemiFuture<bool>(std::move(err));
}

} // namespace rk::projects::durable_log::client