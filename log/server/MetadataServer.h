//
// Created by Rahul  Kushwaha on 2/26/23.
//
#pragma once

#include <grpc++/grpc++.h>
#include "log/server/proto/MetadataService.grpc.pb.h"
#include "log/server/proto/Common.pb.h"
#include "../include/MetadataStore.h"

namespace rk::projects::durable_log::server {

class MetadataServer final: public server::MetadataService::Service {
 public:
  ~MetadataServer() override;

  grpc::Status getConfig(::grpc::ServerContext *context,
                         const MetadataVersionId *request,
                         MetadataConfig *response) override;

  grpc::Status getConfigUsingLogId(::grpc::ServerContext *context,
                                   const LogId *request,
                                   MetadataConfig *response) override;

  grpc::Status compareAndAppendRange(::grpc::ServerContext *context,
                                     const CompareAndAppendRangeRequest *request,
                                     ::google::protobuf::Empty *response) override;


 private:
  std::shared_ptr<MetadataStore> metadataStore_;
};

}