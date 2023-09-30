//
// Created by Rahul  Kushwaha on 7/7/23.
//
#pragma once

#include "log/include/MetadataStore.h"
#include "statemachine/MetadataStoreApplicator.h"
#include "statemachine/MetadataStoreStateMachine.h"
#include "wor/include/WriteOnceRegisterChain.h"
#include "wor/paxos/include/Registry.h"

namespace rk::projects::durable_log {
std::shared_ptr<state_machine::MetadataStoreStateMachine>
makeMetadataStoreStateMachine(MetadataConfig bootstrapConfig,
                              std::shared_ptr<MetadataStore> metadataStore,
                              std::shared_ptr<paxos::Registry> registry) {
  auto currentConfigSupplier =
      [metadataStore]() -> folly::SemiFuture<MetadataConfig> {
    return metadataStore->getCurrentVersionId().semi().deferValue(
        [metadataStore](auto&& value) {
          folly::SemiFuture<MetadataConfig> f =
              metadataStore->getConfig(value).semi().deferValue(
                  [](std::optional<MetadataConfig>&& optionalConfig) {
                    if (optionalConfig.has_value()) {
                      return optionalConfig.value();
                    }

                    throw std::runtime_error{"metadata config not found"};
                  });

          return std::move(f);
        });
  };

  auto applicator = std::make_shared<state_machine::MetadataStoreApplicator>(
      std::move(metadataStore));
  auto stateMachine =
      std::make_shared<state_machine::MetadataStoreStateMachine>(
          std::move(applicator), std::move(bootstrapConfig),
          std::move(currentConfigSupplier), std::move(registry));
  return stateMachine;
}

class PersistentMetadataStore : public MetadataStore {
 public:
  explicit PersistentMetadataStore(MetadataConfig bootstrapConfig,
                                   std::shared_ptr<MetadataStore> metadataStore,
                                   std::shared_ptr<paxos::Registry> registry)
      : stateMachine_{makeMetadataStoreStateMachine(
            std::move(bootstrapConfig), metadataStore, std::move(registry))},
        delegate_{std::move(metadataStore)} {}

  coro<std::optional<MetadataConfig>> getConfig(VersionId versionId) override {
    co_await stateMachine_->sync();
    co_return co_await delegate_->getConfig(versionId);
  }

  coro<std::optional<MetadataConfig>> getConfigUsingLogId(
      LogId logId) override {
    co_await stateMachine_->sync();
    co_return co_await delegate_->getConfigUsingLogId(logId);
  }

  coro<VersionId> getCurrentVersionId() override {
    co_await stateMachine_->sync();
    co_return co_await delegate_->getCurrentVersionId();
  }

  coro<void> compareAndAppendRange(MetadataConfig newMetadataConfig) override {
    auto result = co_await stateMachine_->append(std::move(newMetadataConfig));
    if (std::holds_alternative<std::true_type>(result)) {
      co_return;
    }

    if (std::holds_alternative<state_machine::ApplicationErrors>(result)) {
      auto ex = std::get<state_machine::ApplicationErrors>(result);

      throw std::get<OptimisticConcurrencyException>(ex);
    }

    throw std::get<folly::exception_wrapper>(result);
  }

  void printConfigChain() override {
    stateMachine_->sync().semi().get();
    delegate_->printConfigChain();
  }

  ~PersistentMetadataStore() override = default;

 private:
  std::shared_ptr<state_machine::MetadataStoreStateMachine> stateMachine_;
  std::shared_ptr<MetadataStore> delegate_;
};

}  // namespace rk::projects::durable_log
