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
  auto metadataConfig = metadataStore_->getConfig(request->id());
  if (metadataConfig.has_value()) {
    response->CopyFrom(*metadataConfig);
  } else {
    return grpc::Status{grpc::StatusCode::INTERNAL, "Config not found"};
  }

  return grpc::Status::OK;
}

grpc::Status MetadataServer::getConfigUsingLogId(::grpc::ServerContext *context,
                                                 const server::LogId *request,
                                                 MetadataConfig *response) {
  auto metadataConfig = metadataStore_->getConfigUsingLogId(request->id());
  if (metadataConfig.has_value()) {
    response->CopyFrom(*metadataConfig);
  } else {
    return grpc::Status{grpc::StatusCode::INTERNAL, "Config not found"};
  }

  return grpc::Status::OK;
}

grpc::Status
MetadataServer::compareAndAppendRange(::grpc::ServerContext *context,
                                      const server::CompareAndAppendRangeRequest *request,
                                      ::google::protobuf::Empty *response) {
  try {
    metadataStore_->compareAndAppendRange(request->metadata_version_id().id(),
                                          request->metadata_config());
  } catch (const OptimisticConcurrencyException &e) {
    return grpc::Status{grpc::StatusCode::INTERNAL,
                        "Optimistic Concurrency Control healthy"};
  }

  return grpc::Status::OK;
}

MetadataServer::~MetadataServer() = default;

}
