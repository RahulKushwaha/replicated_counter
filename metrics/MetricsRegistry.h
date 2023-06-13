//
// Created by Rahul  Kushwaha on 3/25/23.
//
#pragma once

#include "log/server/proto/ServerConfig.pb.h"
#include "prometheus/registry.h"

namespace rk::projects::metrics {

using namespace prometheus;

class MetricsRegistry {
public:
  explicit MetricsRegistry() : registry_{std::make_shared<Registry>()} {}

  static MetricsRegistry instance() {
    static MetricsRegistry metricsEmitter{};

    return metricsEmitter;
  }

  Registry &registry() { return *registry_; }

  std::weak_ptr<Registry> registryWeakRef() { return registry_; }

private:
  std::shared_ptr<Registry> registry_;
};
} // namespace rk::projects::metrics