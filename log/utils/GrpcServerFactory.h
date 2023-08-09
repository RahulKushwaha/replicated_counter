//
// Created by Rahul  Kushwaha on 3/5/23.
//
#pragma once
#include <folly/executors/GlobalExecutor.h>
#include <folly/futures/Future.h>
#include <grpc++/grpc++.h>
#include <memory>

namespace rk::projects::durable_log::server {

folly::SemiFuture<std::shared_ptr<grpc::Server>>
runRPCServer(std::string serverAddress,
             const std::vector<grpc::Service *> &grpcServices,
             folly::Executor *executor, const std::string &name = {}) {
  folly::Promise<std::shared_ptr<grpc::Server>> promise;
  folly::SemiFuture<std::shared_ptr<grpc::Server>> future =
      promise.getSemiFuture();

  executor->add([serverAddress = std::move(serverAddress),
                 promise = std::move(promise), name, grpcServices]() mutable {
    // Build server
    grpc::ServerBuilder builder;
    builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
    for (auto grpcService : grpcServices) {
      builder.RegisterService(grpcService);
    }
    std::shared_ptr<grpc::Server> server{builder.BuildAndStart()};
    promise.setValue(server);

    LOG(INFO) << "Name:" << name << " Server listening on " << serverAddress
              << std::endl;
    server->Wait();
  });

  return future;
}

} // namespace rk::projects::durable_log::server