#include <iostream>
#include <utility>

#include "applications/counter/CounterApp.h"
#include "applications/counter/server/CounterAppServer.h"
#include "applications/counter/client/CounterAppClient.h"
#include "log/impl/VirtualLogFactory.h"
#include "applications/CounterAppEnsembleNode.h"
#include <folly/executors/CPUThreadPoolExecutor.h>

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
    std::shared_ptr<VirtualLog> log = makeVirtualLog("CounterApplication");
    auto counterApp = std::make_shared<CounterApp>(log);

    std::string serverAddress{"localhost:8080"};
    auto future = runServer(serverAddress, counterApp);

    CounterAppClient client{
        grpc::CreateChannel(serverAddress,
                            grpc::InsecureChannelCredentials())};

    std::int64_t val = 0;
    for (int i = 1; i <= 50; i++) {
      LOG(INFO) << (val = client.incrementAndGet(1));
    }

    for (int i = 1; i <= 50; i++) {
      LOG(INFO) << (val = client.decrementAndGet(1));
    }

    assert(val == 0);
    assert(val == client.getValue());

    future.value()->Shutdown();
  }


  {
    LOG(INFO) << SERVER_NAME;
    folly::CPUThreadPoolExecutor threadPoolExecutor{16};
    LOG(INFO) << "Created a threadPool with size: "
              << threadPoolExecutor.numThreads();

    auto counterAppEnsemble =
        makeCounterAppEnsemble("CounterApp", &threadPoolExecutor);

    LOG(INFO) << "\n\nSequencers and Replica stared\n\n";
    auto &app1 = counterAppEnsemble.nodes_[0].app;

    std::int64_t val = 0;
    for (int i = 1; i <= 50; i++) {
      LOG(INFO) << (val = app1->incrementAndGet(1));
    }

    auto &app2 = counterAppEnsemble.nodes_[0].app;
    for (int i = 1; i <= 50; i++) {
      LOG(INFO) << (val = app2->decrementAndGet(1));
    }

    assert(val == 0);
    for (int i = 0; i < 5; i++) {
      assert(counterAppEnsemble.nodes_[i].app->getValue() == 0);
    }

    LOG(INFO) << "Every Replica has the same value: " << val;
  }

  return 0;
}
