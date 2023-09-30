//
// Created by Rahul  Kushwaha on 3/19/23.
//

#include "CounterHealthCheck.h"

#include "metrics/MetricsRegistry.h"
#include "prometheus/counter.h"

namespace rk::projects::counter_app {

auto& healthCheck =
    prometheus::BuildCounter()
        .Name("health_check")
        .Help("Health Check")
        .Register(metrics::MetricsRegistry::instance().registry());

auto& healthCheckSuccess = healthCheck.Add({{"result", "success"}});
auto& healthCheckFailure = healthCheck.Add({{"result", "failure"}});

CounterHealthCheck::CounterHealthCheck(std::shared_ptr<CounterApp> counterApp)
    : app_{std::move(counterApp)} {}

folly::coro::Task<bool> CounterHealthCheck::isAlive() {
  try {
    auto key = std::string{KEY_NAME};
    co_await app_->incrementAndGet(key, 1);
    co_await app_->decrementAndGet(key, 1);

    healthCheckSuccess.Increment();
  } catch (const std::exception& e) {
    LOG(ERROR) << "Health Check for App failed: " << e.what();
    healthCheckFailure.Increment();
    co_return false;
  }
  co_return true;
}

}  // namespace rk::projects::counter_app