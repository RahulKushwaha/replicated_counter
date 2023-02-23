//
// Created by Rahul  Kushwaha on 2/23/23.
//

#include "ReplicaServer.h"

namespace rk::projects::durable_log {

ReplicaServer::ReplicaServer(std::shared_ptr<Replica> replica)
    : replica_{std::move(replica)} {}

grpc::Status
ReplicaServer::getId(::grpc::ServerContext *context,
                     const ::google::protobuf::Empty *request,
                     server::IdResponse *response) {
  response->set_id(replica_->getId());
  return grpc::Status::OK;
}

grpc::Status
ReplicaServer::append(::grpc::ServerContext *context,
                      const server::ReplicaAppendRequest *request,
                      ::google::protobuf::Empty *response) {
  replica_->append(request->log_id(), request->payload(), request->skip_seal())
      .get();

  return grpc::Status::OK;
}

grpc::Status
ReplicaServer::getLogEntry(::grpc::ServerContext *context,
                           const server::GetLogEntryRequest *request,
                           server::GetLogEntryResponse *response) {
  auto logEntryResponse = replica_->getLogEntry(request->log_id()).get();
  if (std::holds_alternative<LogEntry>(logEntryResponse)) {
    LogEntry logEntry = std::get<LogEntry>(logEntryResponse);
    response->mutable_log_entry()->set_log_id(logEntry.logId);
    response->mutable_log_entry()->set_payload(logEntry.payload);
  } else {
    LogReadError readError = std::get<LogReadError>(logEntryResponse);
    switch (readError) {
      case LogReadError::NotFound:
        response->set_log_read_error(server::GetLogEntryResponse_LOG_READ_ERROR::GetLogEntryResponse_LOG_READ_ERROR_NOT_FOUND);
        break;

      case LogReadError::IndexOutOfBounds:
        response->set_log_read_error(server::GetLogEntryResponse_LOG_READ_ERROR::GetLogEntryResponse_LOG_READ_ERROR_INDEX_OUT_OF_BOUNDS);
        break;

      default:
        response->set_log_read_error(server::GetLogEntryResponse_LOG_READ_ERROR::GetLogEntryResponse_LOG_READ_ERROR_UNKNOWN);
    }
  }

  return grpc::Status::OK;
}

grpc::Status
ReplicaServer::getLocalCommitIndex(::grpc::ServerContext *context,
                                   const ::google::protobuf::Empty *request,
                                   server::LogIdResponse *response) {
  response->set_log_id(replica_->getLocalCommitIndex());
  return grpc::Status::OK;
}

grpc::Status
ReplicaServer::seal(::grpc::ServerContext *context,
                    const server::SealRequest *request,
                    server::LogIdResponse *response) {
  response->set_log_id(replica_->seal(request->version_id()));
  return grpc::Status::OK;
}

ReplicaServer::~ReplicaServer() = default;

}