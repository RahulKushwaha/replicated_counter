//
// Created by Rahul  Kushwaha on 2/23/23.
//
#include "applications/counter/CounterApp.h"
#include "applications/counter/server/proto/CounterService.grpc.pb.h"
#include <folly/executors/GlobalExecutor.h>
#include <grpc++/grpc++.h>

namespace rk::projects::counter_app {

class CounterAppService final : public CounterService::Service {
public:
  explicit CounterAppService(std::shared_ptr<CounterApp> counterApp)
      : counterApp_{std::move(counterApp)} {}

  grpc::Status IncrementAndGet(::grpc::ServerContext *context,
                               const IncrementRequest *request,
                               CounterValue *response) override {
    LOG(INFO) << "Server Request: IncrementAndGet";

    response->set_value(
        counterApp_->incrementAndGet(request->key(), request->incr_by())
            .semi()
            .get());
    response->set_key(request->key());

    return grpc::Status::OK;
  }

  grpc::Status DecrementAndGet(::grpc::ServerContext *context,
                               const DecrementRequest *request,
                               CounterValue *response) override {
    LOG(INFO) << "Server Request: DecrementAndGet";

    response->set_value(
        counterApp_->decrementAndGet(request->key(), request->decr_by())
            .semi()
            .get());
    response->set_key(request->key());

    return grpc::Status::OK;
  }

  grpc::Status GetCounterValue(::grpc::ServerContext *context,
                               const GetCounterValueRequest *request,
                               CounterValue *response) override {
    LOG(INFO) << "Server Request: GetCounterValue";

    response->set_value(counterApp_->getValue(request->key()).semi().get());
    response->set_key(request->key());

    return grpc::Status::OK;
  }

  grpc::Status BatchUpdate(::grpc::ServerContext *context,
                           const BatchUpdateRequest *request,
                           BatchUpdateResponse *response) override {
    LOG(INFO) << "Server Request: BatchUpdate";
    // TODO(Rahul): Add batch operation here
    return grpc::Status::OK;
  }

  ~CounterAppService() override = default;

private:
  std::shared_ptr<CounterApp> counterApp_;
};

folly::SemiFuture<std::shared_ptr<grpc::Server>>
runServer(const std::string &serverAddress,
          std::shared_ptr<CounterApp> counterApp) {
  folly::Promise<std::shared_ptr<grpc::Server>> promise;
  folly::SemiFuture<std::shared_ptr<grpc::Server>> future =
      promise.getSemiFuture();

  folly::getGlobalCPUExecutor()->add(
      [serverAddress, promise = std::move(promise),
       counterApp = std::move(counterApp)]() mutable {
        CounterAppService service{counterApp};

        // Build server
        grpc::ServerBuilder builder;
        builder.AddListeningPort(serverAddress,
                                 grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        std::shared_ptr<grpc::Server> server{builder.BuildAndStart()};
        promise.setValue(server);

        LOG(INFO) << "Server listening on " << serverAddress << std::endl;
        server->Wait();
      });

  return future;
}

} // namespace rk::projects::counter_app