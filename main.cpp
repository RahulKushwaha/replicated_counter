#include <iostream>
#include <utility>

#include "applications/counter/CounterApp.h"
#include "log/impl/VirtualLogFactory.h"

int main() {
  using namespace rk::projects::durable_log;
  using namespace rk::projects::counter_app;

  LOG(INFO) << "Init Counter Application" << std::endl;


  std::shared_ptr<VirtualLog> log = makeVirtualLog("CounterApplication");
  auto counterApp = CounterApp(log);

  LOG(INFO) << "Counter Application Started";
  srand(time(nullptr));

  std::int64_t val = 0;
  for (int i = 1; i <= 50; i++) {
    LOG(INFO) << (val = counterApp.incrementAndGet(1));
  }

  for (int i = 1; i <= 50; i++) {
    LOG(INFO) << (val = counterApp.decrementAndGet(1));
  }

  assert(val == 0);

  return 0;
}
