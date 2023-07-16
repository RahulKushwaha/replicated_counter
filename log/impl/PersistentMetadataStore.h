//
// Created by Rahul  Kushwaha on 7/7/23.
//
#pragma once

#include "log/include/MetadataStore.h"
#include "statemachine/MetadataStoreApplicator.h"
#include "statemachine/MetadataStoreStateMachine.h"
#include "wor/include/WriteOnceRegisterChain.h"

namespace rk::projects::durable_log {
std::shared_ptr<state_machine::MetadataStoreStateMachine>
makeMetadataStoreStateMachine(std::shared_ptr<MetadataStore> metadataStore) {
  auto currentConfigSupplier =
      [metadataStore]() -> folly::SemiFuture<MetadataConfig> {
    return metadataStore->getCurrentVersionId().semi().deferValue(
        [metadataStore](auto &&value) {
          folly::SemiFuture<MetadataConfig> f =
              metadataStore->getConfig(value).semi().deferValue(
                  [](std::optional<MetadataConfig> &&optionalConfig) {
                    if (optionalConfig.has_value()) {
                      return optionalConfig.value();
                    }

                    throw std::runtime_error{"metadata config not found"};
                  });

          return std::move(f);
        });
  };
  auto bootstrapConfigId = metadataStore->getCurrentVersionId().semi().get();
  auto bootstrapConfig =
      metadataStore->getConfig(bootstrapConfigId).semi().get();
  assert(bootstrapConfig.has_value());

  auto applicator = std::make_shared<state_machine::MetadataStoreApplicator>(
      std::move(metadataStore));
  auto stateMachine =
      std::make_shared<state_machine::MetadataStoreStateMachine>(
          std::move(applicator), std::move(bootstrapConfig.value()),
          std::move(currentConfigSupplier));
  return stateMachine;
}

class PersistentMetadataStore : public MetadataStore {
public:
  explicit PersistentMetadataStore(std::shared_ptr<MetadataStore> metadataStore)
      : stateMachine_{makeMetadataStoreStateMachine(metadataStore)},
        delegate_{std::move(metadataStore)} {}

  coro<std::optional<MetadataConfig>> getConfig(VersionId versionId) override {
    return delegate_->getConfig(versionId);
  }

  coro<std::optional<MetadataConfig>>
  getConfigUsingLogId(LogId logId) override {
    return delegate_->getConfigUsingLogId(logId);
  }

  coro<VersionId> getCurrentVersionId() override {
    return delegate_->getCurrentVersionId();
  }

  coro<void> compareAndAppendRange(MetadataConfig newMetadataConfig) override {
    co_await stateMachine_->append(std::move(newMetadataConfig));
  }

  void printConfigChain() override { delegate_->printConfigChain(); }

  ~PersistentMetadataStore() override = default;

private:
  std::shared_ptr<state_machine::MetadataStoreStateMachine> stateMachine_;
  std::shared_ptr<MetadataStore> delegate_;
};

} // namespace rk::projects::durable_log
