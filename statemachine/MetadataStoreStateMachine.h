//
// Created by Rahul  Kushwaha on 7/8/23.
//
#pragma once
#include "MetadataStoreApplicator.h"
#include "statemachine/Common.h"
#include "statemachine/include/StateMachine.h"
#include "wor/WORFactory.h"
#include "wor/WriteOnceRegisterChainAppender.h"
#include "wor/include/WriteOnceRegister.h"
#include "wor/paxos/include/Registry.h"

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
      currentConfigSupplier_t currentConfigSupplier,
      std::shared_ptr<paxos::Registry> registry)
      : mtx_{std::make_unique<std::mutex>()},
        applicator_{std::move(applicator)},
        currentConfig_{std::move(bootstrapConfig)},
        lastAppliedWorId_{currentConfig_.start_index()},
        currentConfigSupplier_{std::move(currentConfigSupplier)},
        registry_{std::move(registry)} {}

  folly::coro::Task<ApplicationResult>
  append(durable_log::MetadataConfig config) override {
    auto payload = config.SerializeAsString();

    bool successfullyWritten{false};
    ApplicationResult result;

    while (!successfullyWritten) {
      auto nextWorId = lastAppliedWorId_ + 1;

      // TODO(Rahul): Stop making vector every time.
      auto wor = wor::makePaxosWor(nextWorId,
                                   {currentConfig_.replica_set_config().begin(),
                                    currentConfig_.replica_set_config().end()},
                                   registry_);
      bool worIterationComplete{false};
      while (!worIterationComplete) {
        auto lockId = co_await wor->lock();
        if (!lockId.has_value()) {
          continue;
        }

        auto writeResponse = co_await wor->write(lockId.value(), payload);
        if (writeResponse) {
          successfullyWritten = true;
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

        result = co_await apply(metadataConfig);
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

  folly::coro::Task<ApplicationResult>
  apply(durable_log::MetadataConfig config) override {
    co_return co_await applicator_->apply(std::move(config));
  }

  // A better way to implement sync is to contact quorum.
  coro<void> sync() override {
    while (true) {
      auto nextWorId = lastAppliedWorId_ + 1;
      LOG(INFO) << "next wor id: " << nextWorId;
      auto wor = wor::makePaxosWor(nextWorId,
                                   {currentConfig_.replica_set_config().begin(),
                                    currentConfig_.replica_set_config().end()},
                                   registry_);

      auto readResult = co_await wor->read();
      if (std::holds_alternative<wor::WriteOnceRegister::ReadError>(
              readResult)) {
        auto readError =
            std::get<wor::WriteOnceRegister::ReadError>(readResult);

        if (readError == wor::WriteOnceRegister::ReadError::NOT_WRITTEN) {
          co_return;
        }

        LOG(INFO) << "wor status could not be determined. continue probing";
      } else {
        auto worValue = std::get<std::string>(readResult);
        auto metadataConfig = durable_log::MetadataConfig{};
        bool parseResult = metadataConfig.ParseFromString(worValue);
        if (!parseResult) {
          throw std::runtime_error{"protobuf parse error"};
        }

        co_await apply(metadataConfig);
        currentConfig_ = co_await currentConfigSupplier_();
        lastAppliedWorId_ = nextWorId;
      }
    }
  }

  void setApplicator(std::shared_ptr<
                     Applicator<durable_log::MetadataConfig, ApplicationResult>>
                         applicator) override {
    applicator_ = std::move(applicator);
  }

private:
  // TODO(rahul): Refactor out lock. Handle somewhere else.
  std::unique_ptr<std::mutex> mtx_;
  durable_log::MetadataConfig currentConfig_;
  wor::WorId lastAppliedWorId_;
  std::shared_ptr<applicator_t> applicator_;
  currentConfigSupplier_t currentConfigSupplier_;
  std::shared_ptr<paxos::Registry> registry_;
};

} // namespace rk::projects::state_machine