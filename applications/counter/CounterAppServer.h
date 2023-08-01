//
// Created by Rahul  Kushwaha on 3/28/23.
//
#include <memory>

#include "applications/counter/CounterApp.h"
#include "applications/counter/CounterAppStateMachine.h"
#include "applications/counter/CounterApplicator.h"
#include "applications/counter/CounterHealthCheck.h"
#include "log/impl/FailureDetectorImpl.h"
#include "log/server/LogServer.h"
#include "persistence/KVStoreLite.h"
#include "persistence/RocksDbFactory.h"
#include "statemachine/Factory.h"
#include "statemachine/LogTrimApplicator.h"
#include "statemachine/LogTrimStateMachine.h"
#include "statemachine/VirtualLogStateMachine.h"
#include "statemachine/include/StateMachine.h"

namespace rk::projects::counter_app {

using StateMachineReturnType =
    std::variant<std::vector<CounterKeyValue>, folly::Unit>;
using namespace state_machine;

class CounterAppServer {
public:
  explicit CounterAppServer(CounterAppConfig appConfig,
                            rk::projects::server::ServerConfig logServerConfig)
      : appConfig_{std::move(appConfig)},
        logServerConfig_{std::move(logServerConfig)} {}

  folly::coro::Task<void> start() {
    State state{.logServerConfig = logServerConfig_};

    state.logServer =
        std::make_shared<rk::projects::durable_log::server::LogServer>(
            state.logServerConfig);

    co_await state.logServer->start();
    LOG(INFO) << "Starting App";

    auto rocks = persistence::RocksDbFactory::provideSharedPtr(
        persistence::RocksDbFactory::RocksDbConfig{
            .path = appConfig_.data_directory(),
            .createIfMissing = true,
            .manualWALFlush = true,
        });

    state.kvStore =
        std::make_shared<persistence::RocksKVStoreLite>(std::move(rocks));

    auto stateMachineStack = makeStateMachineStack<StateMachineReturnType>(
        nullptr, state.logServer->getVirtualLog());

    state.appStateMachine =
        std::make_shared<CounterAppStateMachine>(stateMachineStack);

    state.counterApp =
        std::make_shared<CounterApp>(state.appStateMachine, state.kvStore);

    state.counterApplicator =
        std::make_shared<CounterApplicator>(state.counterApp);

    stateMachineStack->setUpstreamStateMachine(state.appStateMachine);

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

  folly::coro::Task<void> stop() { co_return; }

private:
  struct State {
    rk::projects::server::ServerConfig logServerConfig;
    std::shared_ptr<rk::projects::durable_log::server::LogServer> logServer;

    std::shared_ptr<persistence::KVStoreLite> kvStore;

    std::shared_ptr<CounterApp> counterApp;
    std::shared_ptr<CounterAppStateMachine> appStateMachine;
    std::shared_ptr<CounterApplicator> counterApplicator;

    std::shared_ptr<folly::CPUThreadPoolExecutor> failureDetectorPool;
    std::shared_ptr<FailureDetector> failureDetector;
  };

  CounterAppConfig appConfig_;
  rk::projects::server::ServerConfig logServerConfig_;
  std::shared_ptr<State> state_;
};

} // namespace rk::projects::counter_app