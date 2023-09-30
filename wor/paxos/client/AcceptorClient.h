//
// Created by Rahul  Kushwaha on 7/22/23.
//
#pragma once
#include "wor/paxos/include/Acceptor.h"
#include "wor/paxos/server/proto/Acceptor.grpc.pb.h"

#include <grpc++/grpc++.h>

namespace rk::projects::paxos::client {
using namespace std::chrono_literals;
constexpr auto CLIENT_TIMEOUT = 250ms;

class AcceptorClient {
 public:
  explicit AcceptorClient(std::shared_ptr<grpc::Channel> channel)
      : stub_{server::Acceptor::NewStub(std::move(channel))} {}

  std::string getId() {
    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);
    server::IdResponse response{};

    auto status = stub_->getId(
        &context, google::protobuf::Empty::default_instance(), &response);

    if (status.ok()) {
      return response.id();
    }

    throw std::runtime_error(context.debug_error_string());
  }

  coro<std::variant<Promise, std::false_type>> prepare(
      std::string paxosInstanceId, Ballot ballot) {
    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);

    server::PrepareRequest prepareRequest{};
    prepareRequest.set_paxos_instance_id(std::move(paxosInstanceId));
    *prepareRequest.mutable_ballot_id() = std::move(ballot.id);
    server::PrepareResponse prepareResponse{};

    auto status = stub_->prepare(&context, prepareRequest, &prepareResponse);
    if (status.ok()) {
      co_return {};
    }

    auto err = folly::make_exception_wrapper<std::runtime_error>(
        context.debug_error_string());
    co_return co_await folly::makeSemiFuture<
        std::variant<Promise, std::false_type>>(std::move(err));
  }

  coro<bool> accept(std::string paxosInstanceId, Proposal proposal) {
    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);

    server::AcceptRequest request{};
    request.set_paxos_instance_id(paxosInstanceId);
    request.mutable_proposal()->CopyFrom(proposal);

    server::AcceptResponse response{};

    auto status = stub_->accept(&context, request, &response);
    if (status.ok()) {
      co_return response.result();
    }

    auto err = folly::make_exception_wrapper<std::runtime_error>(
        context.debug_error_string());
    co_return co_await folly::makeSemiFuture<bool>(std::move(err));
  }

  coro<bool> commit(std::string paxosInstanceId, BallotId ballotId) {
    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);

    server::CommitRequest request{};
    request.set_paxos_instance_id(std::move(paxosInstanceId));
    request.mutable_ballot_id()->CopyFrom(ballotId);

    server::CommitResponse response{};

    auto status = stub_->commit(&context, request, &response);
    if (status.ok()) {
      co_return response.result();
    }

    auto err = folly::make_exception_wrapper<std::runtime_error>(
        context.debug_error_string());
    co_return co_await folly::makeSemiFuture<bool>(std::move(err));
  }

  coro<std::optional<Promise>> getAcceptedValue(std::string paxosInstanceId) {
    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);

    server::GetAcceptedValueRequest request{};
    request.set_paxos_instance_id(std::move(paxosInstanceId));

    server::GetAcceptedValueResponse response{};

    auto status = stub_->getAcceptedValue(&context, request, &response);
    if (status.ok()) {
      if (response.has_promise()) {
        co_return response.promise();
      }

      co_return {};
    }

    auto err = folly::make_exception_wrapper<std::runtime_error>(
        context.debug_error_string());
    co_return co_await folly::makeSemiFuture<std::optional<Promise>>(
        std::move(err));
  }

  coro<std::optional<std::string>> getCommittedValue(
      std::string paxosInstanceId) {
    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);

    server::GetCommittedValueRequest request{};
    request.set_paxos_instance_id(std::move(paxosInstanceId));

    server::GetCommittedValueResponse response{};

    auto status = stub_->getCommittedValue(&context, request, &response);
    if (status.ok()) {
      if (response.has_value()) {
        co_return response.value();
      }

      co_return {};
    }

    auto err = folly::make_exception_wrapper<std::runtime_error>(
        context.debug_error_string());
    co_return co_await folly::makeSemiFuture<std::optional<std::string>>(
        std::move(err));
  }

 private:
  std::unique_ptr<server::Acceptor::Stub> stub_;
};

}  // namespace rk::projects::paxos::client