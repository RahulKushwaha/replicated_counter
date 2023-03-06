//
// Created by Rahul  Kushwaha on 3/5/23.
//
#include <memory>
#include <grpc++/grpc++.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/futures/Future.h>

namespace rk::projects::durable_log::server {

folly::SemiFuture<std::shared_ptr<grpc::Server>>
runRPCServer(std::string serverAddress, grpc::Service *grpcService,
             folly::Executor *executor, const std::string &name = {}) {
  folly::Promise<std::shared_ptr<grpc::Server>> promise;
  folly::SemiFuture<std::shared_ptr<grpc::Server>>
      future = promise.getSemiFuture();

  executor->add(
      [serverAddress = std::move(serverAddress),
          promise = std::move(promise), name,
          grpcService]()mutable {
        // Build server
        grpc::ServerBuilder builder;
        builder.AddListeningPort(serverAddress,
                                 grpc::InsecureServerCredentials());
        builder.RegisterService(grpcService);
        std::shared_ptr<grpc::Server> server{builder.BuildAndStart()};
        promise.setValue(server);

        LOG(INFO) << "Name:" << name << "Server listening on " << serverAddress
                  << std::endl;
        server->Wait();
      });

  return future;
}

}