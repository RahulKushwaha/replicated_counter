//
// Created by Rahul  Kushwaha on 9/9/23.
//
#pragma once
#include "log/proto/LogEntry.pb.h"
#include "statemachine/include/StateMachine.h"

namespace rk::projects::state_machine {

using namespace rk::projects::durable_log;
using in_t = LogEntry_1;
using out_t = LogEntry_1;

class SinkStateMachine : public StateMachine<in_t, out_t> {
public:
  explicit SinkStateMachine(
      std::shared_ptr<StateMachine<in_t, out_t>> downstreamStateMachine)
      : applicator_{nullptr},
        downstreamStateMachine_{std::move(downstreamStateMachine)},
        upstreamStateMachine_{nullptr}, entries_{} {}

  coro<out_t> append(in_t input) override {
    return downstreamStateMachine_->append(std::move(input));
  }

  coro<out_t> apply(in_t input) override {
    LOG(INFO) << "received: " << input.single_log_entry().id();
    entries_.emplace_back(input);
    co_return input;
  }

  virtual coro<void> sync() override {
    co_return co_await downstreamStateMachine_->sync();
  }

  void
  setApplicator(std::shared_ptr<Applicator<in_t, out_t>> applicator) override {
    applicator_ = std::move(applicator);
  }

  void setUpstreamStateMachine(std::shared_ptr<StateMachine<in_t, out_t>>
                                   upstreamStateMachine) override {
    upstreamStateMachine_ = std::move(upstreamStateMachine);
  }

  std::vector<LogEntry_1> getAllLogEntries() { return entries_; }

private:
  std::shared_ptr<Applicator<in_t, out_t>> applicator_;
  std::shared_ptr<StateMachine<in_t, out_t>> downstreamStateMachine_;
  std::shared_ptr<StateMachine<in_t, out_t>> upstreamStateMachine_;
  std::vector<LogEntry_1> entries_;
};

} // namespace rk::projects::state_machine
