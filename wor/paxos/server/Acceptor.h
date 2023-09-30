//
// Created by Rahul  Kushwaha on 7/20/23.
//
#pragma once
#include "wor/paxos/include/Acceptor.h"
#include "wor/paxos/server/proto/Acceptor.grpc.pb.h"

namespace rk::projects::paxos::server {

class AcceptorServer : public Acceptor::Service {
 public:
  explicit AcceptorServer(std::shared_ptr<paxos::Acceptor> acceptor)
      : acceptor_{std::move(acceptor)} {}

  grpc::Status getId(::grpc::ServerContext* context,
                     const ::google::protobuf::Empty* request,
                     IdResponse* response) override {
    response->set_id(acceptor_->getId());
    return Service::getId(context, request, response);
  }

  grpc::Status prepare(::grpc::ServerContext* context,
                       const PrepareRequest* request,
                       PrepareResponse* response) override {
    auto prepareResult = acceptor_
                             ->prepare(request->paxos_instance_id(),
                                       Ballot{request->ballot_id()})
                             .semi()
                             .get();

    if (std::holds_alternative<std::false_type>(prepareResult)) {
      response->mutable_prepare_failed()->CopyFrom(PrepareFailed{});
    } else {
      response->mutable_promise()->CopyFrom(std::get<Promise>(prepareResult));
    }

    return grpc::Status::OK;
  }

  grpc::Status accept(::grpc::ServerContext* context,
                      const AcceptRequest* request,
                      AcceptResponse* response) override {
    auto result =
        acceptor_->accept(request->paxos_instance_id(), request->proposal())
            .semi()
            .get();
    response->set_result(result);

    return grpc::Status::OK;
  }

  grpc::Status commit(::grpc::ServerContext* context,
                      const CommitRequest* request,
                      CommitResponse* response) override {
    auto result =
        acceptor_->commit(request->paxos_instance_id(), request->ballot_id())
            .semi()
            .get();
    response->set_result(result);

    return grpc::Status::OK;
  }

  grpc::Status getAcceptedValue(::grpc::ServerContext* context,
                                const GetAcceptedValueRequest* request,
                                GetAcceptedValueResponse* response) override {
    auto result =
        acceptor_->getAcceptedValue(request->paxos_instance_id()).semi().get();

    if (result.has_value()) {
      response->mutable_promise()->CopyFrom(result.value());
    }

    return grpc::Status::OK;
  }

  grpc::Status getCommittedValue(::grpc::ServerContext* context,
                                 const GetCommittedValueRequest* request,
                                 GetCommittedValueResponse* response) override {
    auto result =
        acceptor_->getCommittedValue(request->paxos_instance_id()).semi().get();

    if (result.has_value()) {
      response->set_value(std::move(result.value()));
    }

    return grpc::Status::OK;
  }

  ~AcceptorServer() override = default;

 private:
  std::shared_ptr<paxos::Acceptor> acceptor_;
};

}  // namespace rk::projects::paxos::server