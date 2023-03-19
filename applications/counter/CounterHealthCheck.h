//
// Created by Rahul  Kushwaha on 3/19/23.
//
#pragma once

#include "log/include/HealthCheck.h"
#include "CounterApp.h"

namespace rk::projects::counter_app {

class CounterHealthCheck: public durable_log::HealthCheck {
 public:
  explicit CounterHealthCheck(std::shared_ptr<CounterApp> counterApp);

 public:
  folly::coro::Task<bool> isAlive() override;

 private:
  std::shared_ptr<CounterApp> app_;
  static constexpr std::string_view KEY_NAME = "TEST_KEY";
};

}
