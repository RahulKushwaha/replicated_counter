//
// Created by Rahul  Kushwaha on 3/28/23.
//
#include <memory>

#include "applications/counter/CounterApp.h"
#include "applications/counter/CounterHealthCheck.h"
#include "log/impl/FailureDetectorImpl.h"
#include "log/server/LogServer.h"

namespace rk::projects::counter_app {

class CounterAppServer {
public:
  explicit CounterAppServer(rk::projects::server::ServerConfig logServerConfig)
      : logServerConfig_{std::move(logServerConfig)} {}

  folly::coro::Task<void> start() {
    State state{.logServerConfig = logServerConfig_};

    state.logServer =
        std::make_shared<rk::projects::durable_log::server::LogServer>(
            state.logServerConfig);

    co_await state.logServer->start();
    LOG(INFO) << "Starting App";
    state.counterApp =
        std::make_shared<CounterApp>(state.logServer->getVirtualLog());

    state.failureDetectorPool =
        std::make_shared<folly::CPUThreadPoolExecutor>(4);

    std::shared_ptr<HealthCheck> healthCheck =
        std::make_shared<CounterHealthCheck>(state.counterApp);

    state.failureDetector = std::make_shared<FailureDetectorImpl>(
        std::move(healthCheck), state.logServer->getVirtualLog(),
        state.failureDetectorPool, state.logServerConfig);

    state_ = std::make_shared<State>(std::move(state));
    co_return;
  }

  folly::coro::Task<void> stop() {}

private:
private:
  struct State {
    rk::projects::server::ServerConfig logServerConfig;
    std::shared_ptr<rk::projects::durable_log::server::LogServer> logServer;
    std::shared_ptr<CounterApp> counterApp;

    std::shared_ptr<folly::CPUThreadPoolExecutor> failureDetectorPool;
    std::shared_ptr<FailureDetector> failureDetector;
  };

  rk::projects::server::ServerConfig logServerConfig_;
  std::shared_ptr<State> state_;
};

} // namespace rk::projects::counter_app