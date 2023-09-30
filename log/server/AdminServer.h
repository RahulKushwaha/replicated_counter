
//
// Created by Rahul  Kushwaha on 3/13/23.
//

#pragma once

#include "log/include/MetadataStore.h"
#include "log/include/VirtualLog.h"
#include "log/server/proto/Admin.grpc.pb.h"

#include <grpc++/grpc++.h>

namespace rk::projects::durable_log::server {

class AdminServer final : public AdminService::Service {
 public:
  explicit AdminServer(std::shared_ptr<VirtualLog> virtualLog);
  grpc::Status getCurrentConfig(
      ::grpc::ServerContext* context, const ::google::protobuf::Empty* request,
      ::rk::projects::durable_log::MetadataConfig* response) override;
  grpc::Status reconfigure(::grpc::ServerContext* context,
                           const ReconfigurationRequest* request,
                           ReconfigurationResponse* response) override;

  ~AdminServer() override = default;

 private:
  std::shared_ptr<VirtualLog> virtualLog_;
};

}  // namespace rk::projects::durable_log::server
