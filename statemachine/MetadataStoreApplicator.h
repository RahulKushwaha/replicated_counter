//
// Created by Rahul  Kushwaha on 7/8/23.
//
#pragma once
#include "folly/experimental/coro/Task.h"
#include "log/include/MetadataStore.h"
#include "log/proto/MetadataConfig.pb.h"
#include "statemachine/include/StateMachine.h"

namespace rk::projects::state_machine {

class MetadataStoreApplicator
    : public Applicator<durable_log::MetadataConfig, void> {
public:
  explicit MetadataStoreApplicator(
      std::shared_ptr<durable_log::MetadataStore> metadataStore)
      : metadataStore_{std::move(metadataStore)} {}

  folly::coro::Task<void> apply(durable_log::MetadataConfig metadataConfig) override {
    co_await metadataStore_->compareAndAppendRange(metadataConfig);
  }

private:
  std::shared_ptr<durable_log::MetadataStore> metadataStore_;
};

} // namespace rk::projects::state_machine
