//
// Created by Rahul  Kushwaha on 7/16/23.
//
#pragma once
#include "log/proto/LogEntry.pb.h"
#include "statemachine/NullStateMachine.h"
#include "statemachine/include/StateMachine.h"

namespace rk::projects::state_machine {

using namespace rk::projects::durable_log;

template <typename R>
class LogTrimStateMachine : public StateMachine<LogEntry_1, R> {
public:
  explicit LogTrimStateMachine(
      std::shared_ptr<Applicator<LogEntry_1, R>> applicator,
      std::shared_ptr<StateMachine<LogEntry_1, R>> downstreamStateMachine)
      : applicator_{std::move(applicator)},
        downstreamStateMachine_{std::move(downstreamStateMachine)},
        upstreamStateMachine_{
            std::make_shared<NullStateMachine<LogEntry_1, R>>()} {}

  coro<R> append(rk::projects::durable_log::LogEntry_1 t) override {
    co_return co_await downstreamStateMachine_->append(std::move(t));
  }

  coro<R> apply(rk::projects::durable_log::LogEntry_1 t) override {
    if (t.has_log_trim_entry()) {
      co_return co_await applicator_->apply(std::move(t));
    }

    co_return co_await upstreamStateMachine_->apply(std::move(t));
  }

  coro<void> sync() override {
    co_return co_await downstreamStateMachine_->sync();
  }

  void setApplicator(
      std::shared_ptr<Applicator<LogEntry_1, R>> applicator) override {
    applicator_ = std::move(applicator);
  }

  void setUpstreamStateMachine(std::shared_ptr<StateMachine<LogEntry_1, R>>
                                   upstreamStateMachine) override {
    upstreamStateMachine_ = std::move(upstreamStateMachine);
  }

private:
  std::shared_ptr<Applicator<LogEntry_1, R>> applicator_;
  std::shared_ptr<StateMachine<LogEntry_1, R>> downstreamStateMachine_;
  std::shared_ptr<StateMachine<LogEntry_1, R>> upstreamStateMachine_;
};

} // namespace rk::projects::state_machine
