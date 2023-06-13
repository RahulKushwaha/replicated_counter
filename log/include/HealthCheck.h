//
// Created by Rahul  Kushwaha on 3/19/23.
//

#pragma once

#include "folly/experimental/coro/Task.h"

namespace rk::projects::durable_log {

class HealthCheck {
public:
  virtual folly::coro::Task<bool> isAlive() = 0;
};

} // namespace rk::projects::durable_log
