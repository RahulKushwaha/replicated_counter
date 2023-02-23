//
// Created by Rahul  Kushwaha on 2/23/23.
//

#include "SequencerServer.h"
namespace rk::projects::durable_log::server {

SequencerServer::SequencerServer(std::shared_ptr<Sequencer> sequencer)
    : sequencer_{std::move(sequencer)} {}

grpc::Status SequencerServer::getId(::grpc::ServerContext *context,
                                    const ::google::protobuf::Empty *request,
                                    IdResponse *response) {
  response->set_id(sequencer_->getId());
  return grpc::Status::OK;
}

grpc::Status SequencerServer::append(::grpc::ServerContext *context,
                                     const SequencerAppendRequest *request,
                                     LogIdResponse *response) {
  auto logId = sequencer_->append(request->payload()).get();
  response->set_log_id(logId);

  return grpc::Status::OK;
}

grpc::Status
SequencerServer::latestAppendPosition(::grpc::ServerContext *context,
                                      const ::google::protobuf::Empty *request,
                                      LogIdResponse *response) {
  response->set_log_id(sequencer_->latestAppendPosition().get());

  return grpc::Status::OK;
}

grpc::Status SequencerServer::isAlive(::grpc::ServerContext *context,
                                      const ::google::protobuf::Empty *request,
                                      IsAliveResponse *response) {
  response->set_is_alive(sequencer_->isAlive());
  return grpc::Status::OK;
}

SequencerServer::~SequencerServer() = default;

}