//
// Created by Rahul  Kushwaha on 2/26/23.
//
#pragma once

#include "log/include/MetadataStore.h"
#include "log/server/proto/Common.pb.h"
#include "log/server/proto/MetadataService.grpc.pb.h"

#include <grpc++/grpc++.h>

namespace rk::projects::durable_log::server {

class MetadataServer final : public server::MetadataService::Service {
 public:
  explicit MetadataServer(std::shared_ptr<MetadataStore> metadataStore);
  grpc::Status getConfig(::grpc::ServerContext* context,
                         const MetadataVersionId* request,
                         MetadataConfig* response) override;

  grpc::Status getConfigUsingLogId(::grpc::ServerContext* context,
                                   const LogId* request,
                                   MetadataConfig* response) override;

  grpc::Status getCurrentConfig(::grpc::ServerContext* context,
                                const ::google::protobuf::Empty* request,
                                MetadataConfig* response) override;

  grpc::Status compareAndAppendRange(
      ::grpc::ServerContext* context,
      const CompareAndAppendRangeRequest* request,
      ::google::protobuf::Empty* response) override;

  grpc::Status printConfigChain(::grpc::ServerContext* context,
                                const ::google::protobuf::Empty* request,
                                ::google::protobuf::Empty* response) override;

  ~MetadataServer() override;

 private:
  std::shared_ptr<MetadataStore> metadataStore_;
};

}  // namespace rk::projects::durable_log::server