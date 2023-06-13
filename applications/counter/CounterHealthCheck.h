//
// Created by Rahul  Kushwaha on 3/19/23.
//
#pragma once

#include "CounterApp.h"
#include "log/include/HealthCheck.h"
#include "log/utils/UuidGenerator.h"

namespace rk::projects::counter_app {

class CounterHealthCheck : public durable_log::HealthCheck {
public:
  explicit CounterHealthCheck(std::shared_ptr<CounterApp> counterApp);

public:
  folly::coro::Task<bool> isAlive() override;

private:
  std::shared_ptr<CounterApp> app_;
  std::string KEY_NAME{"HEALTH_CHECK_" +
                       utils::UuidGenerator::instance().generate()};
};

} // namespace rk::projects::counter_app
