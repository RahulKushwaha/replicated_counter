//
// Created by rahul on 12/22/23.
//

#pragma once
#include "applications/mydb/client/proto/db.grpc.pb.h"

namespace rk::projects::mydb {

class DbServer : public client::db::Service {
 public:
  DbServer() {}

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
  grpc::Status scanTable(grpc::ServerContext* context,
                         const client::ScanTableRequest* request,
                         client::ScanTableResponse* response) override;

 private:
};

}  // namespace rk::projects::mydb