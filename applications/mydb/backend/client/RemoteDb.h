//
// Created by Rahul  Kushwaha on 2/13/24.
//

#pragma once
#include "applications/mydb/client/proto/db.grpc.pb.h"
#include "applications/mydb/include/Db.h"

#include <grpc++/grpc++.h>

namespace rk::projects::mydb::client {

using namespace std::chrono_literals;
constexpr auto CLIENT_TIMEOUT = 250ms;

class RemoteDb : public Db {
 public:
  explicit RemoteDb(std::shared_ptr<grpc::Channel> channel)
      : stub_{db::NewStub(std::move(channel))} {}

  coro<google::protobuf::Empty> addDatabase(
      const client::AddDatabaseRequest* request) override {
    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);
    google::protobuf::Empty response{};

    auto status = stub_->addDatabase(&context, *request, &response);
    if (status.ok()) {
      co_return {};
    }

    throw std::runtime_error(context.debug_error_string());
  }

  coro<google::protobuf::Empty> addTable(
      const client::AddTableRequest* request) override {
    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);
    google::protobuf::Empty response{};

    auto status = stub_->addTable(&context, *request, &response);
    if (status.ok()) {
      co_return {};
    }

    throw std::runtime_error(context.debug_error_string());
  }

  coro<AddRowResponse> addRow(const client::AddRowRequest* request) override {
    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);
    AddRowResponse response{};

    auto status = stub_->addRow(&context, *request, &response);
    if (status.ok()) {
      co_return response;
    }

    throw std::runtime_error(context.debug_error_string());
  }

  coro<UpdateRowResponse> updateRow(
      const client::UpdateRowRequest* request) override {
    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);
    UpdateRowResponse response{};

    auto status = stub_->updateRow(&context, *request, &response);
    if (status.ok()) {
      co_return response;
    }

    throw std::runtime_error(context.debug_error_string());
  }

  coro<GetRowResponse> getRow(const client::GetRowRequest* request) override {
    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);
    GetRowResponse response{};

    auto status = stub_->getRows(&context, *request, &response);

    if (status.ok()) {
      co_return response;
    }

    throw std::runtime_error(context.debug_error_string());
  }

  coro<MultiTableOperationResponse> multiTableOperation(
      const client::MultiTableOperationRequest* request) override {
    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);
    MultiTableOperationResponse response{};

    auto status = stub_->multiTableOperation(&context, *request, &response);

    if (status.ok()) {
      co_return response;
    }

    throw std::runtime_error(context.debug_error_string());
  }

  coro<ScanTableResponse> scanTable(
      const client::ScanTableRequest* request) override {
    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);
    ScanTableResponse response{};

    auto status = stub_->scanTable(&context, *request, &response);

    if (status.ok()) {
      co_return response;
    }

    throw std::runtime_error(context.debug_error_string());
  }

  coro<PrepareTransactionResponse> prepareTransaction(
      const client::PrepareTransactionRequest* request) override {
    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);
    PrepareTransactionResponse response{};

    auto status = stub_->prepareTransaction(&context, *request, &response);

    if (status.ok()) {
      co_return response;
    }

    throw std::runtime_error(context.debug_error_string());
  }

  coro<CommitTransactionResponse> commitTransaction(
      const client::CommitTransactionRequest* request) override {
    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);
    CommitTransactionResponse response{};

    auto status = stub_->commitTransaction(&context, *request, &response);

    if (status.ok()) {
      co_return response;
    }

    throw std::runtime_error(context.debug_error_string());
  }

 private:
  std::unique_ptr<db::Stub> stub_;
};

}  // namespace rk::projects::mydb::client