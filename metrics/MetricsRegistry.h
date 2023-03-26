//
// Created by Rahul  Kushwaha on 3/25/23.
//
#pragma once
#include "prometheus/exposer.h"
#include "prometheus/registry.h"

namespace rk::projects::metrics {

using namespace prometheus;

class MetricsRegistry {
 public:
  explicit MetricsRegistry() :
      registry_{std::make_shared<Registry>()},
      exposer_{std::make_shared<Exposer>("127.0.0.1:9999")} {
    // ask the exposer to scrape the registry on incoming HTTP requests
    exposer_->RegisterCollectable(registry_);
  }

  static MetricsRegistry instance() {
    static MetricsRegistry metricsEmitter{};

    return metricsEmitter;
  }

  Registry &registry() {
    return *registry_;
  }

 private:

  std::shared_ptr<Registry> registry_;
  std::shared_ptr<Exposer> exposer_;
};
}