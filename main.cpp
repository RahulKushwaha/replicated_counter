#include <iostream>
#include <utility>

#include "include/DurableLog.h"
#include "VectorBasedDurableLog.h"
#include "Counter.h"
#include "CounterApplication.h"

int main() {
  using namespace rk::project::counter;

  std::cout << "Init Counter Application" << std::endl;

  std::cout << "Init Durable Log" << std::endl;
  std::shared_ptr<DurableLog>
      log = std::make_shared<VectorBasedDurableLog>("COUNTER", 0);

  std::cout << "Init Counter Applicator" << std::endl;
  std::shared_ptr<Counter> counter = std::make_shared<Counter>(0, log, -1);

  CounterApplication counterApplication{log, counter};
  srand(time(nullptr));

  for (int i = 0; i < 100; i++) {
    auto val = rand() % 100;
    std::cout << counterApplication.increment(val) << std::endl;
  }

  return 0;
}
