#include <iostream>
#include <utility>

#include "applications/counter/CounterAppEnsembleNode.h"
#include "applications/counter/CounterApp.h"
#include "applications/counter/client/CounterAppClient.h"
#include "applications/counter/server/CounterAppServer.h"
#include "log/impl/VirtualLogFactory.h"
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/experimental/coro/BlockingWait.h>
#include <folly/experimental/coro/Task.h>

const char *SERVER_NAME = R"(
  ____            _ _           _           _   _
 |  _ \ ___ _ __ | (_) ___ __ _| |_ ___  __| | | |    ___   __ _
 | |_) / _ \ '_ \| | |/ __/ _` | __/ _ \/ _` | | |   / _ \ / _` |
 |  _ <  __/ |_) | | | (_| (_| | ||  __/ (_| | | |__| (_) | (_| |
 |_| \_\___| .__/|_|_|\___\__,_|\__\___|\__,_| |_____\___/ \__, |
           |_|                                             |___/
)";

int main() {
  using namespace rk::projects::durable_log;
  using namespace rk::projects::counter_app;

  {
    std::string key{"TEST_KEY"};
    std::shared_ptr<VirtualLog> log = makeVirtualLog("CounterApplication");
    auto counterApp = std::make_shared<CounterApp>(log);

    std::string serverAddress{"localhost:8080"};
    auto future = runServer(serverAddress, counterApp);

    CounterAppClient client{
        grpc::CreateChannel(serverAddress,
                            grpc::InsecureChannelCredentials())};

    std::int64_t val = 0;
    for (int i = 1; i <= 50; i++) {
      LOG(INFO) << (val = client.incrementAndGet(key, 1).semi().get());
    }

    for (int i = 1; i <= 50; i++) {
      LOG(INFO) << (val = client.decrementAndGet(key, 1).semi().get());
    }

    assert(val == 0);
    assert(val == client.getValue(key).semi().get());

    future.value()->Shutdown();
  }


  {
    std::string key{"TEST_KEY"};
    LOG(INFO) << SERVER_NAME;
    folly::CPUThreadPoolExecutor threadPoolExecutor{16};
    LOG(INFO) << "Created a threadPool with size: "
              << threadPoolExecutor.numThreads();

    auto counterAppEnsemble =
        makeCounterAppEnsemble("CounterApp", &threadPoolExecutor);

    LOG(INFO) << "\n\nSequencers and Replica stared\n\n";
    auto &ensembleNode = counterAppEnsemble.nodes_[0];

    bool healthy = false;
    while (!healthy) {
      bool currentState = true;
      folly::futures::sleep(std::chrono::milliseconds{500}).get();
      LOG(INFO) << "Waiting for server to be alive before sending request";

      std::int32_t index = 0;

      for (auto &ensemble: counterAppEnsemble.nodes_) {
        auto ensembleAlive = ensemble.failureDetector->healthy();
        LOG(INFO) << "Ensemble Node: " << index << " "
                  << (ensembleAlive ? "T" : "F");

        currentState &= ensembleAlive;
        index++;
      }

      healthy = currentState;
    }

    auto &app1 = counterAppEnsemble.nodes_[0].app;


    std::int64_t val = 0;
    for (int i = 1; i <= 50; i++) {
      LOG(INFO) << (val = app1->incrementAndGet(key, 1).semi().get());
    }

    auto &app2 = counterAppEnsemble.nodes_[0].app;
    for (int i = 1; i <= 50; i++) {
      LOG(INFO) << (val = app2->decrementAndGet(key, 1).semi().get());
    }

    assert(val == 0);
    for (int i = 0; i < 5; i++) {
      assert(counterAppEnsemble.nodes_[i].app->getValue(key).semi().get() == 0);
    }

    LOG(INFO) << "Every Replica has the same value: " << val;

//    while (true) {
//      LOG(INFO) << "Sleeping for 2 seconds";
//      folly::futures::sleep(std::chrono::milliseconds{20000}).get();
//    }
  }

  return 0;
}
