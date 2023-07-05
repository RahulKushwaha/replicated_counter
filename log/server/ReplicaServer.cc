//
// Created by Rahul  Kushwaha on 2/23/23.
//

#include "ReplicaServer.h"

namespace rk::projects::durable_log::server {

ReplicaServer::ReplicaServer(std::shared_ptr<Replica> replica)
    : replica_{std::move(replica)} {}

grpc::Status ReplicaServer::getId(::grpc::ServerContext *context,
                                  const ::google::protobuf::Empty *request,
                                  server::IdResponse *response) {
  try {
    response->set_id(replica_->getId());
  } catch (const std::exception &e) {
    return grpc::Status{grpc::StatusCode::UNKNOWN, e.what()};
  }

  return grpc::Status::OK;
}

grpc::Status ReplicaServer::append(::grpc::ServerContext *context,
                                   const server::ReplicaAppendRequest *request,
                                   ::google::protobuf::Empty *response) {
  try {
    auto globalCommitIndex =
        request->has_global_commit_index()
            ? std::optional<LogId>{request->global_commit_index()}
            : std::optional<LogId>{};
    replica_
        ->append(globalCommitIndex, request->version_id(), request->log_id(),
                 request->payload(), request->skip_seal())
        .semi()
        .get();
  } catch (const std::exception &e) {
    return grpc::Status{grpc::StatusCode::UNKNOWN, e.what()};
  }

  return grpc::Status::OK;
}

grpc::Status
ReplicaServer::getLogEntry(::grpc::ServerContext *context,
                           const server::GetLogEntryRequest *request,
                           server::GetLogEntryResponse *response) {
  try {
    auto logEntryResponse =
        replica_->getLogEntry(request->version_id(), request->log_id())
            .semi()
            .get();

    if (std::holds_alternative<durable_log::LogEntry>(logEntryResponse)) {
      auto logEntry = std::get<durable_log::LogEntry>(logEntryResponse);
      response->mutable_log_entry()->set_log_id(logEntry.logId);
      response->mutable_log_entry()->set_payload(logEntry.payload);
    } else {
      LogReadError readError = std::get<LogReadError>(logEntryResponse);
      switch (readError) {
      case LogReadError::NotFound:
        response->set_log_read_error(
            server::GetLogEntryResponse_LOG_READ_ERROR::
                GetLogEntryResponse_LOG_READ_ERROR_NOT_FOUND);
        break;

      case LogReadError::IndexOutOfBounds:
        response->set_log_read_error(
            server::GetLogEntryResponse_LOG_READ_ERROR::
                GetLogEntryResponse_LOG_READ_ERROR_INDEX_OUT_OF_BOUNDS);
        break;

      default:
        response->set_log_read_error(
            server::GetLogEntryResponse_LOG_READ_ERROR::
                GetLogEntryResponse_LOG_READ_ERROR_UNKNOWN);
      }
    }

  } catch (const std::exception &e) {
    return grpc::Status{grpc::StatusCode::UNKNOWN, e.what()};
  }

  return grpc::Status::OK;
}

grpc::Status ReplicaServer::getLocalCommitIndex(
    ::grpc::ServerContext *context,
    const server::GetLocalCommitIndexRequest *request,
    server::LogIdResponse *response) {
  try {
    response->set_log_id(
        replica_->getCommitIndex(request->version_id()).semi().get());
  } catch (const std::exception &e) {
    return grpc::Status{grpc::StatusCode::UNKNOWN, e.what()};
  }

  return grpc::Status::OK;
}

grpc::Status ReplicaServer::seal(::grpc::ServerContext *context,
                                 const server::SealRequest *request,
                                 server::LogIdResponse *response) {
  try {
    response->set_log_id(replica_->seal(request->version_id()).semi().get());
  } catch (const std::exception &e) {
    return grpc::Status{grpc::StatusCode::UNKNOWN, e.what()};
  }

  return grpc::Status::OK;
}

ReplicaServer::~ReplicaServer() = default;

} // namespace rk::projects::durable_log::server