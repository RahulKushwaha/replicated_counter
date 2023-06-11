//
// Created by Rahul  Kushwaha on 6/9/23.
//
#pragma once
#include "statemachine/Common.h"
#include "statemachine/include/StateMachine.h"
#include "wor/WriteOnceRegisterChainAppender.h"

namespace rk::projects::state_machine {

class RocksStateMachine : public StateMachine<RocksTxn, RocksTxnResult> {
private:
  using applicator_t = Applicator<RocksTxn, RocksTxnResult>;
  using appender_t = wor::WriteOnceRegisterChainAppender<std::string>;

public:
  explicit RocksStateMachine(std::shared_ptr<applicator_t> applicator,
                             std::shared_ptr<wor::WriteOnceRegisterChain> chain)
      : lastAppliedWorId_{0}, applicator_{std::move(applicator)},
        chain_{std::move(chain)},
        appender_{std::make_shared<appender_t>(chain_)} {}

  folly::coro::Task<RocksTxnResult> append(RocksTxn txn) override {
    auto toWorId = co_await appender_->append(txn.serializedPayload);

    for (wor::WorId i = lastAppliedWorId_ + 1; i < toWorId; i++) {
      auto wor = chain_->get(i);
      // remove this in future as all the wors' should be completely written.
      assert(wor.has_value());

      auto serializedPayload = std::get<std::string>(wor.value()->read());
      RocksTxn currentTxn{.serializedPayload = serializedPayload};

      co_await applicator_->apply(currentTxn);
    }

    co_return co_await applicator_->apply(txn);
  }

  void setApplicator(std::shared_ptr<applicator_t> applicator) override {
    applicator_ = std::move(applicator);
  }

private:
  wor::WorId lastAppliedWorId_;
  std::shared_ptr<applicator_t> applicator_;
  std::shared_ptr<wor::WriteOnceRegisterChain> chain_;
  std::shared_ptr<appender_t> appender_;
};

} // namespace rk::projects::state_machine