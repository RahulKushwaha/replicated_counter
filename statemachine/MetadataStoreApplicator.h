//
// Created by Rahul  Kushwaha on 7/8/23.
//
#pragma once
#include "folly/experimental/coro/Task.h"
#include "log/include/MetadataStore.h"
#include "log/proto/MetadataConfig.pb.h"
#include "statemachine/include/StateMachine.h"

namespace rk::projects::state_machine {

using ApplicationErrors =
    std::variant<durable_log::OptimisticConcurrencyException>;
using ApplicationResult =
    std::variant<std::true_type, ApplicationErrors, folly::exception_wrapper>;

class MetadataStoreApplicator
    : public Applicator<durable_log::MetadataConfig, ApplicationResult> {
public:
  explicit MetadataStoreApplicator(
      std::shared_ptr<durable_log::MetadataStore> metadataStore)
      : metadataStore_{std::move(metadataStore)} {}

  folly::coro::Task<ApplicationResult>
  apply(durable_log::MetadataConfig metadataConfig) override {
    try {
      co_await metadataStore_->compareAndAppendRange(std::move(metadataConfig));
      co_return std::true_type{};
    } catch (durable_log::OptimisticConcurrencyException &e) {
      LOG(INFO) << e.what();
      co_return ApplicationErrors{std::move(e)};
    } catch (...) {
      auto exceptionWrapper =
          folly::exception_wrapper{std::current_exception()};
      LOG(INFO) << exceptionWrapper.what();
      co_return exceptionWrapper;
    }
  }

private:
  std::shared_ptr<durable_log::MetadataStore> metadataStore_;
};

} // namespace rk::projects::state_machine
