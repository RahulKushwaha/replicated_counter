#include <iostream>
#include <utility>

#include "applications/counter/CounterApp.h"
#include "applications/counter/server/CounterAppServer.h"
#include "applications/counter/client/CounterAppClient.h"
#include "log/impl/VirtualLogFactory.h"

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

  return 0;
}
