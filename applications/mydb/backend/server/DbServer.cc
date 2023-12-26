//
// Created by rahul on 12/22/23.
//

#include "DbServer.h"

namespace rk::projects::mydb {
grpc::Status DbServer::addDatabase(grpc::ServerContext* context,
                                   const client::AddDatabaseRequest* request,
                                   google::protobuf::Empty* response) {
  return grpc::Status::OK;
}

grpc::Status DbServer::addTable(grpc::ServerContext* context,
                                const client::AddTableRequest* request,
                                google::protobuf::Empty* response) {
  return grpc::Status::OK;
}

grpc::Status DbServer::addRow(grpc::ServerContext* context,
                              const client::AddRowRequest* request,
                              client::AddRowResponse* response) {
  return grpc::Status::OK;
}

grpc::Status DbServer::scanTable(grpc::ServerContext* context,
                                 const client::ScanTableRequest* request,
                                 client::ScanTableResponse* response) {
  return grpc::Status::OK;
}

}  // namespace rk::projects::mydb