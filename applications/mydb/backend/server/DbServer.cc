//
// Created by rahul on 12/22/23.
//

#include "applications/mydb/backend/server/DbServer.h"

#include "applications/mydb/backend/Errors.h"

namespace rk::projects::mydb {
grpc::Status DbServer::addDatabase(grpc::ServerContext* context,
                                   const client::AddDatabaseRequest* request,
                                   google::protobuf::Empty* response) {
  try {
    *response = db_->addDatabase(request).semi().get();
  } catch (const DbError& error) {
    return grpc::Status{grpc::StatusCode::UNKNOWN, error.what()};
  }

  return grpc::Status::OK;
}

grpc::Status DbServer::addTable(grpc::ServerContext* context,
                                const client::AddTableRequest* request,
                                google::protobuf::Empty* response) {
  try {
    *response = db_->addTable(request).semi().get();
  } catch (const DbError& error) {
    return grpc::Status{grpc::StatusCode::UNKNOWN, error.what()};
  }

  return grpc::Status::OK;
}

grpc::Status DbServer::addRow(grpc::ServerContext* context,
                              const client::AddRowRequest* request,
                              client::AddRowResponse* response) {
  try {
    *response = db_->addRow(request).semi().get();
  } catch (const DbError& error) {
    return grpc::Status{grpc::StatusCode::UNKNOWN, error.what()};
  }

  return grpc::Status::OK;
}

::grpc::Status DbServer::updateRow(::grpc::ServerContext* context,
                                   const client::UpdateRowRequest* request,
                                   client::UpdateRowResponse* response) {
  try {
    *response = db_->updateRow(request).semi().get();
  } catch (const DbError& error) {
    return grpc::Status{grpc::StatusCode::UNKNOWN, error.what()};
  }

  return grpc::Status::OK;
}

::grpc::Status DbServer::getRows(::grpc::ServerContext* context,
                                 const client::GetRowRequest* request,
                                 client::GetRowResponse* response) {
  return Service::getRows(context, request, response);
}

::grpc::Status DbServer::multiTableOperation(
    ::grpc::ServerContext* context,
    const client::MultiTableOperationRequest* request,
    client::MultiTableOperationResponse* response) {
  try {
    *response = db_->multiTableOperation(request).semi().get();
  } catch (const DbError& error) {
    return grpc::Status{grpc::StatusCode::UNKNOWN, error.what()};
  }

  return grpc::Status::OK;
}

::grpc::Status DbServer::prepareTransaction(
    ::grpc::ServerContext* context,
    const client::PrepareTransactionRequest* request,
    client::PrepareTransactionResponse* response) {
  try {
    *response = db_->prepareTransaction(request).semi().get();
  } catch (const DbError& error) {
    return grpc::Status{grpc::StatusCode::UNKNOWN, error.what()};
  }

  return grpc::Status::OK;
}

::grpc::Status DbServer::commitTransaction(
    ::grpc::ServerContext* context,
    const client::CommitTransactionRequest* request,
    client::CommitTransactionResponse* response) {
  return Service::commitTransaction(context, request, response);
}

grpc::Status DbServer::scanTable(grpc::ServerContext* context,
                                 const client::ScanTableRequest* request,
                                 client::ScanTableResponse* response) {
  try {
    *response = db_->scanTable(request).semi().get();
  } catch (const DbError& error) {
    return grpc::Status{grpc::StatusCode::UNKNOWN, error.what()};
  }

  return grpc::Status::OK;
}

grpc::Status DbServer::abortTransaction(
    ::grpc::ServerContext* context,
    const ::rk::projects::mydb::client::AbortTransactionRequest* request,
    ::rk::projects::mydb::client::AbortTransactionResponse* response) {
  try {
    *response = db_->abortTransaction(request).semi().get();
  } catch (const DbError& error) {
    return grpc::Status{grpc::StatusCode::UNKNOWN, error.what()};
  }

  return grpc::Status::OK;
}

DbServer::~DbServer() = default;
}  // namespace rk::projects::mydb