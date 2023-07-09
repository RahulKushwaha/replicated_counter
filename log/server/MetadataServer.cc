//
// Created by Rahul  Kushwaha on 2/26/23.
//

#include "MetadataServer.h"

namespace rk::projects::durable_log::server {

MetadataServer::MetadataServer(std::shared_ptr<MetadataStore> metadataStore)
    : metadataStore_{std::move(metadataStore)} {}

grpc::Status MetadataServer::getConfig(::grpc::ServerContext *context,
                                       const server::MetadataVersionId *request,
                                       MetadataConfig *response) {
  auto metadataConfig = metadataStore_->getConfig(request->id()).semi().get();
  if (metadataConfig.has_value()) {
    response->CopyFrom(*metadataConfig);
  } else {
    return grpc::Status{grpc::StatusCode::INTERNAL, "config not found"};
  }

  return grpc::Status::OK;
}

grpc::Status MetadataServer::getCurrentConfig(
    ::grpc::ServerContext *context, const ::google::protobuf::Empty *request,
    ::rk::projects::durable_log::MetadataConfig *response) {
  auto versionId = metadataStore_->getCurrentVersionId().semi().get();
  auto metadataConfig = metadataStore_->getConfig(versionId).semi().get();
  if (metadataConfig.has_value()) {
    response->CopyFrom(*metadataConfig);
  } else {
    LOG(INFO) << "config not found";
    return grpc::Status{grpc::StatusCode::INTERNAL, "config not found"};
  }

  return grpc::Status::OK;
}

grpc::Status MetadataServer::getConfigUsingLogId(::grpc::ServerContext *context,
                                                 const server::LogId *request,
                                                 MetadataConfig *response) {
  auto metadataConfig =
      metadataStore_->getConfigUsingLogId(request->id()).semi().get();
  if (metadataConfig.has_value()) {
    response->CopyFrom(*metadataConfig);
  } else {
    return grpc::Status{grpc::StatusCode::INTERNAL, "config not Round"};
  }

  return grpc::Status::OK;
}

grpc::Status MetadataServer::compareAndAppendRange(
    ::grpc::ServerContext *context,
    const server::CompareAndAppendRangeRequest *request,
    ::google::protobuf::Empty *response) {
  try {
    metadataStore_->compareAndAppendRange(request->metadata_config())
        .semi()
        .get();
  } catch (const OptimisticConcurrencyException &e) {
    return grpc::Status{grpc::StatusCode::UNKNOWN, e.what()};
  }

  return grpc::Status::OK;
}

grpc::Status
MetadataServer::printConfigChain(::grpc::ServerContext *context,
                                 const ::google::protobuf::Empty *request,
                                 ::google::protobuf::Empty *response) {
  try {
    metadataStore_->printConfigChain();
  } catch (const std::exception &e) {
    return grpc::Status{grpc::StatusCode::UNKNOWN, e.what()};
  }

  return grpc::Status::OK;
}

MetadataServer::~MetadataServer() = default;
} // namespace rk::projects::durable_log::server
