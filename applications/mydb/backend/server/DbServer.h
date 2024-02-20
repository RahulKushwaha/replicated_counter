//
// Created by rahul on 12/22/23.
//

#pragma once
#include "applications/mydb/client/proto/db.grpc.pb.h"
#include "applications/mydb/include/Db.h"

namespace rk::projects::mydb {

class DbServer : public client::db::Service {
 public:
  explicit DbServer(std::shared_ptr<Db> db) : db_{std::move(db)} {}

 public:
  grpc::Status addDatabase(grpc::ServerContext* context,
                           const client::AddDatabaseRequest* request,
                           google::protobuf::Empty* response) override;
  grpc::Status addTable(grpc::ServerContext* context,
                        const client::AddTableRequest* request,
                        google::protobuf::Empty* response) override;
  grpc::Status addRow(grpc::ServerContext* context,
                      const client::AddRowRequest* request,
                      client::AddRowResponse* response) override;
  ::grpc::Status updateRow(::grpc::ServerContext* context,
                           const client::UpdateRowRequest* request,
                           client::UpdateRowResponse* response) override;
  ::grpc::Status getRows(::grpc::ServerContext* context,
                         const client::GetRowRequest* request,
                         client::GetRowResponse* response) override;
  ::grpc::Status multiTableOperation(
      ::grpc::ServerContext* context,
      const client::MultiTableOperationRequest* request,
      client::MultiTableOperationResponse* response) override;

  grpc::Status scanTable(grpc::ServerContext* context,
                         const client::ScanTableRequest* request,
                         client::ScanTableResponse* response) override;

  ::grpc::Status prepareTransaction(
      ::grpc::ServerContext* context,
      const client::PrepareTransactionRequest* request,
      client::PrepareTransactionResponse* response) override;

  ::grpc::Status commitTransaction(
      ::grpc::ServerContext* context,
      const client::CommitTransactionRequest* request,
      client::CommitTransactionResponse* response) override;
  grpc::Status abortTransaction(
      ::grpc::ServerContext* context,
      const ::rk::projects::mydb::client::AbortTransactionRequest* request,
      ::rk::projects::mydb::client::AbortTransactionResponse* response)
      override;

  ~DbServer() override;

 private:
  std::shared_ptr<Db> db_;
};

}  // namespace rk::projects::mydb