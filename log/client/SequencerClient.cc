//
// Created by Rahul  Kushwaha on 2/26/23.
//

#include "SequencerClient.h"


namespace rk::projects::durable_log::client {

SequencerClient::SequencerClient(std::shared_ptr<grpc::Channel> channel) :
    stub_{server::SequencerService::NewStub(std::move(channel))} {}

folly::SemiFuture<std::string> SequencerClient::getId() {
  grpc::ClientContext context;
  google::protobuf::Empty request;
  server::IdResponse response;

  grpc::Status status = stub_->getId(&context, request, &response);

  if (status.ok()) {
    return folly::makeSemiFuture(response.id());
  }

  return folly::makeSemiFuture<std::string>
      (folly::make_exception_wrapper<std::exception>());
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

  return folly::makeSemiFuture<LogId>
      (folly::make_exception_wrapper<std::exception>());
}

folly::SemiFuture<LogId> SequencerClient::latestAppendPosition() {
  grpc::ClientContext context;
  google::protobuf::Empty request;
  server::LogIdResponse response;

  auto status = stub_->latestAppendPosition(&context, request, &response);
  if (status.ok()) {
    return folly::makeSemiFuture<LogId>(response.log_id());
  }

  return folly::makeSemiFuture<LogId>
      (folly::make_exception_wrapper<std::exception>());
}

folly::SemiFuture<bool> SequencerClient::isAlive() {
  grpc::ClientContext context;
  google::protobuf::Empty request;
  server::IsAliveResponse response;

  auto status = stub_->isAlive(&context, request, &response);

  if (status.ok()) {
    return folly::makeSemiFuture<bool>(true);
  }

  return folly::makeSemiFuture<bool>
      (folly::make_exception_wrapper<std::exception>());
}

}