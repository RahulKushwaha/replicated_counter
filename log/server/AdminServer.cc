//
// Created by Rahul  Kushwaha on 3/13/23.
//

#include "AdminServer.h"

namespace rk::projects::durable_log::server {

AdminServer::AdminServer(std::shared_ptr<VirtualLog> virtualLog)
    : virtualLog_{std::move(virtualLog)} {}

grpc::Status AdminServer::getCurrentConfig(::grpc::ServerContext *context,
                                           const ::google::protobuf::Empty *request,
                                           ::rk::projects::durable_log::MetadataConfig *response) {
  response->CopyFrom(virtualLog_->getCurrentConfig().semi().get());
  return grpc::Status::OK;
}

grpc::Status AdminServer::reconfigure(::grpc::ServerContext *context,
                                      const ReconfigurationRequest *request,
                                      ReconfigurationResponse *response) {
  virtualLog_->reconfigure().semi().get();
  return Service::reconfigure(context, request, response);
}

}