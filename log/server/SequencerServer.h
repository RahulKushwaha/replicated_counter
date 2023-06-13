//
// Created by Rahul  Kushwaha on 2/23/23.
//

#pragma once

#include "../include/Sequencer.h"
#include "log/server/proto/Common.pb.h"
#include "log/server/proto/Sequencer.grpc.pb.h"
#include <grpc++/grpc++.h>

namespace rk::projects::durable_log::server {

class SequencerServer final : public SequencerService::Service {
public:
  explicit SequencerServer(std::shared_ptr<Sequencer> sequencer);

  grpc::Status
  getId(::grpc::ServerContext *context,
        const ::google::protobuf::Empty *request,
        ::rk::projects::durable_log::server::IdResponse *response) override;
  grpc::Status
  append(::grpc::ServerContext *context,
         const ::rk::projects::durable_log::server::SequencerAppendRequest
             *request,
         ::rk::projects::durable_log::server::LogIdResponse *response) override;
  grpc::Status latestAppendPosition(
      ::grpc::ServerContext *context, const ::google::protobuf::Empty *request,
      ::rk::projects::durable_log::server::LogIdResponse *response) override;
  grpc::Status isAlive(
      ::grpc::ServerContext *context, const ::google::protobuf::Empty *request,
      ::rk::projects::durable_log::server::IsAliveResponse *response) override;

  ~SequencerServer() override;

private:
  std::shared_ptr<Sequencer> sequencer_;
};

} // namespace rk::projects::durable_log::server