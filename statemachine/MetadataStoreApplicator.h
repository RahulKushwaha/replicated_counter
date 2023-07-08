//
// Created by Rahul  Kushwaha on 7/8/23.
//
#pragma once
#include "folly/experimental/coro/Task.h"
#include "log/include/MetadataStore.h"
#include "statemachine/include/StateMachine.h"

namespace rk::projects::state_machine {

class MetadataStoreApplicator : public Applicator<std::string, void> {
public:
  explicit MetadataStoreApplicator(
      std::shared_ptr<durable_log::MetadataStore> metadataStore)
      : metadataStore_{std::move(metadataStore)} {}

  folly::coro::Task<void> apply(std::string &t) override {
    auto metadataConfig = durable_log::MetadataConfig{};
    bool parseResult = metadataConfig.ParseFromString(t);
    if (!parseResult) {
      throw std::runtime_error{"non recoverable error"};
    }

    co_await metadataStore_->compareAndAppendRange(
        metadataConfig.previous_version_id(), metadataConfig);
  }

private:
  std::shared_ptr<durable_log::MetadataStore> metadataStore_;
};

} // namespace rk::projects::state_machine
