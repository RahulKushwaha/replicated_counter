//
// Created by Rahul  Kushwaha on 3/19/23.
//

#include "CounterHealthCheck.h"

namespace rk::projects::counter_app {

CounterHealthCheck::CounterHealthCheck(std::shared_ptr<CounterApp> counterApp)
    : app_{std::move(counterApp)} {}

folly::coro::Task<bool> CounterHealthCheck::isAlive() {
  try {
    auto key = std::string{KEY_NAME};
    app_->incrementAndGet(key, 1);
    app_->decrementAndGet(key, 1);
  } catch (const std::exception &e) {
    LOG(ERROR) << "Health Check for App failed: " << e.what();
    co_return false;
  }
  co_return true;
}

}