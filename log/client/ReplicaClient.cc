//
// Created by Rahul  Kushwaha on 2/26/23.
//

#include "ReplicaClient.h"

namespace rk::projects::durable_log::client {

using namespace std::chrono_literals;
constexpr auto CLIENT_TIMEOUT = 250ms;

ReplicaClient::ReplicaClient(std::shared_ptr<grpc::Channel> channel) :
    stub_{server::ReplicaService::NewStub(std::move(channel))} {}

folly::SemiFuture<std::string> ReplicaClient::getId() {
  server::IdResponse response;
  grpc::ClientContext context;
  context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);
  google::protobuf::Empty empty;

  grpc::Status status = stub_->getId(&context, empty, &response);
  if (status.ok()) {
    return folly::makeSemiFuture<std::string>(std::string{response.id()});
  }

  return folly::makeSemiFuture<std::string>(folly::make_exception_wrapper<std::exception>());
}

folly::SemiFuture<folly::Unit>
ReplicaClient::append(VersionId versionId, LogId logId,
                      std::string logEntryPayload,
                      bool skipSeal) {
  google::protobuf::Empty response;
  grpc::ClientContext context;
  context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);

  server::ReplicaAppendRequest request;
  request.set_log_id(logId);
  request.set_payload(std::move(logEntryPayload));
  request.set_skip_seal(skipSeal);

  grpc::Status status = stub_->append(&context, request, &response);

  if (status.ok()) {
    return folly::makeSemiFuture();
  }

  auto err =
      folly::make_exception_wrapper<std::runtime_error>(context.debug_error_string());
  return folly::makeSemiFuture<folly::Unit>(std::move(err));
}

folly::SemiFuture<std::variant<LogEntry, LogReadError>>
ReplicaClient::getLogEntry(VersionId versionId, LogId logId) {
  grpc::ClientContext context;
  context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);

  server::GetLogEntryResponse response;
  server::GetLogEntryRequest request;
  request.set_log_id(logId);

  grpc::Status status = stub_->getLogEntry(&context, request, &response);

  std::variant<LogEntry, LogReadError> result;
  if (status.ok()) {
    if (response.has_log_read_error()) {
      switch (response.log_read_error()) {
        case server::GetLogEntryResponse_LOG_READ_ERROR_INDEX_OUT_OF_BOUNDS:
          result = {LogReadError::IndexOutOfBounds};
          break;
        case server::GetLogEntryResponse_LOG_READ_ERROR_NOT_FOUND:
          result = {LogReadError::NotFound};
          break;

        case server::GetLogEntryResponse_LOG_READ_ERROR_UNKNOWN:
        default:
          result = {LogReadError::Unknown};
          break;
      }
    } else {
      result = LogEntry{response.log_entry().log_id(),
                        std::move(*response.mutable_log_entry()->mutable_payload())};
    }

    return folly::makeSemiFuture(result);
  }

  auto err =
      folly::make_exception_wrapper<std::runtime_error>(context.debug_error_string());
  return folly::makeSemiFuture<std::variant<LogEntry, LogReadError>>
      (std::move(err));
}

folly::SemiFuture<LogId>
ReplicaClient::getLocalCommitIndex(VersionId versionId) {
  grpc::ClientContext context;
  context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);

  server::LogIdResponse response;
  server::GetLocalCommitIndexRequest request;
  request.set_version_id(versionId);

  grpc::Status
      status = stub_->getLocalCommitIndex(&context, request, &response);

  if (status.ok()) {
    return folly::makeSemiFuture<LogId>(response.log_id());
  }

  auto err =
      folly::make_exception_wrapper<std::runtime_error>(context.debug_error_string());
  return folly::makeSemiFuture<LogId>(std::move(err));
}

folly::SemiFuture<LogId> ReplicaClient::seal(VersionId versionId) {
  grpc::ClientContext context;
  context.set_deadline(std::chrono::system_clock::now() + CLIENT_TIMEOUT);

  server::LogIdResponse response;
  server::SealRequest sealRequest;
  sealRequest.set_version_id(versionId);

  grpc::Status status = stub_->seal(&context, sealRequest, &response);

  if (status.ok()) {
    return folly::makeSemiFuture(response.log_id());
  }

  auto err =
      folly::make_exception_wrapper<std::runtime_error>(context.debug_error_string());
  return folly::makeSemiFuture<LogId>(std::move(err));
}

}