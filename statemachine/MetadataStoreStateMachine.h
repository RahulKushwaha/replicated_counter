//
// Created by Rahul  Kushwaha on 7/8/23.
//
#pragma once
#include "statemachine/Common.h"
#include "statemachine/include/StateMachine.h"
#include "wor/WORFactory.h"
#include "wor/WriteOnceRegisterChainAppender.h"
#include "wor/include/WriteOnceRegister.h"

namespace rk::projects::state_machine {

class MetadataStoreStateMachine
    : public StateMachine<durable_log::MetadataConfig, ApplicationResult> {
  using applicator_t =
      Applicator<durable_log::MetadataConfig, ApplicationResult>;
  using currentConfigSupplier_t =
      std::function<folly::SemiFuture<durable_log::MetadataConfig>()>;

public:
  explicit MetadataStoreStateMachine(
      std::shared_ptr<applicator_t> applicator,
      durable_log::MetadataConfig bootstrapConfig,
      currentConfigSupplier_t currentConfigSupplier)
      : applicator_{std::move(applicator)},
        currentConfig_{std::move(bootstrapConfig)},
        lastAppliedWorId_{currentConfig_.start_index()},
        currentConfigSupplier_{std::move(currentConfigSupplier)} {}

  folly::coro::Task<ApplicationResult>
  append(durable_log::MetadataConfig config) override {
    auto payload = config.SerializeAsString();

    bool successfullyWritten{false};
    ApplicationResult result;

    while (!successfullyWritten) {
      auto nextWorId = lastAppliedWorId_ + 1;

      auto wor = wor::makePaxosWor(nextWorId, currentConfig_);
      bool worIterationComplete{false};
      while (!worIterationComplete) {
        auto lockId = co_await wor->lock();
        if (!lockId.has_value()) {
          continue;
        }

        auto writeResponse = co_await wor->write(lockId.value(), payload);
        if (writeResponse) {
          successfullyWritten = true;
          break;
        }

        auto readValue = co_await wor->read();
        if (std::holds_alternative<wor::WriteOnceRegister::ReadError>(
                readValue)) {
          LOG(INFO) << "failed to read wor: "
                    << wor::WriteOnceRegister::toString(
                           std::get<wor::WriteOnceRegister::ReadError>(
                               readValue));
          // We either encounter an error or it is not written. In any case
          // we need to continue writing to it until it succeeds.
          continue;
        }

        auto worValue = std::get<std::string>(readValue);
        auto metadataConfig = durable_log::MetadataConfig{};
        bool parseResult = metadataConfig.ParseFromString(worValue);
        if (!parseResult) {
          co_return folly::exception_wrapper{
              std::runtime_error{"protobuf parse error"}};
        }

        result = co_await applicator_->apply(metadataConfig);
        currentConfig_ = co_await currentConfigSupplier_();
        lastAppliedWorId_ = nextWorId;

        if (worValue == payload) {
          successfullyWritten = true;
        }

        worIterationComplete = true;
      }
    }

    co_return result;
  }

  void setApplicator(std::shared_ptr<
                     Applicator<durable_log::MetadataConfig, ApplicationResult>>
                         applicator) override {
    applicator_ = std::move(applicator);
  }

private:
  durable_log::MetadataConfig currentConfig_;
  wor::WorId lastAppliedWorId_;
  std::shared_ptr<applicator_t> applicator_;
  currentConfigSupplier_t currentConfigSupplier_;
};

} // namespace rk::projects::state_machine