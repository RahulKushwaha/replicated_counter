//
// Created by Rahul  Kushwaha on 6/9/23.
//
#pragma once
#include "wor/WriteOnceRegisterChainAppender.h"
#include "statemachine/include/StateMachine.h"

namespace rk::projects::state_machine {

struct RocksTxn {
  std::int64_t txnId;
  std::vector<std::pair<std::string, std::string>> writeKeyValues;
  std::vector<std::string> deleteKeys;
  std::string serializedPayload;
};

class RocksStateMachine: public StateMachine<RocksTxn> {
 public:
  folly::coro::Task<void> append(RocksTxn txn) override {
    auto toWorId = co_await
    appender_->append(txn.serializedPayload);

    for (wor::WorId i = lastAppliedWorId_ + 1; i <= toWorId; i++) {
      auto wor = chain_->get(i);
      // remove this in future as all the wors' should be completely written.
      assert(wor.has_value());

      auto serializedPayload = std::get<std::string>(wor.value()->read());

      co_await
      applicator_->apply(RocksTxn{.serializedPayload = serializedPayload});
    }

    co_return;
  }

  void
  setApplicator(std::shared_ptr<Applicator<RocksTxn>> applicator) override {
    applicator_ = std::move(applicator);
  }

 private:
  wor::WorId lastAppliedWorId_;
  std::shared_ptr<Applicator<RocksTxn>> applicator_;
  std::shared_ptr<wor::WriteOnceRegisterChainAppender<std::string>> appender_;
  std::shared_ptr<wor::WriteOnceRegisterChain> chain_;
};

}