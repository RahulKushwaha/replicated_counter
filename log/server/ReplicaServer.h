//
// Created by Rahul  Kushwaha on 2/23/23.
//
#pragma once

#include <grpc++/grpc++.h>
#include "log/server/proto/Replica.grpc.pb.h"
#include "log/server/proto/Common.pb.h"
#include "../include/Replica.h"

namespace rk::projects::durable_log::server {

class ReplicaServer final: public server::ReplicaService::Service {
 public:
  explicit ReplicaServer(std::shared_ptr<Replica> replica);

  grpc::Status getId(::grpc::ServerContext *context,
                     const ::google::protobuf::Empty *request,
                     server::IdResponse *response) override;
  grpc::Status append(::grpc::ServerContext *context,
                      const server::ReplicaAppendRequest *request,
                      ::google::protobuf::Empty *response) override;
  grpc::Status getLogEntry(::grpc::ServerContext *context,
                           const server::GetLogEntryRequest *request,
                           server::GetLogEntryResponse *response) override;
  grpc::Status getLocalCommitIndex(::grpc::ServerContext *context,
                                   const server::GetLocalCommitIndexRequest *request,
                                   server::LogIdResponse *response) override;
  grpc::Status seal(::grpc::ServerContext *context,
                    const server::SealRequest *request,
                    server::LogIdResponse *response) override;

  ~ReplicaServer() override;

 private:
  std::shared_ptr<Replica> replica_;
};

}
