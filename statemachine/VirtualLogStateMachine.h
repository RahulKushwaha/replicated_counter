//
// Created by Rahul  Kushwaha on 7/17/23.
//
#pragma once
#include "log/include/VirtualLog.h"
#include "log/proto/LogEntry.pb.h"
#include "statemachine/NullStateMachine.h"
#include "statemachine/include/StateMachine.h"

namespace rk::projects::state_machine {

using namespace rk::projects::durable_log;

template <typename R>
class VirtualLogStateMachine : public StateMachine<LogEntry_1, R> {
public:
  explicit VirtualLogStateMachine(std::shared_ptr<VirtualLog> virtualLog,
                                  LogId lastAppliedLogId = 0)
      : virtualLog_{std::move(virtualLog)},
        upstreamStateMachine_{
            std::make_shared<NullStateMachine<LogEntry_1, R>>()},
        lastAppliedLogId_{lastAppliedLogId} {}

  coro<R> append(LogEntry_1 t) override {
    auto logId = co_await virtualLog_->append(t.SerializeAsString());
    for (auto nextLogId = lastAppliedLogId_ + 1; nextLogId <= logId;
         nextLogId++) {
      auto nextLog = co_await virtualLog_->getLogEntry(nextLogId);
      if (std::holds_alternative<durable_log::LogReadError>(nextLog)) {
        auto logReadError = std::get<durable_log::LogReadError>(nextLog);
        throw std::runtime_error{"log read error"};
      }

      durable_log::LogEntry serializedLogEntry = std::get<LogEntry>(nextLog);
      LogEntry_1 logEntry{};
      if (auto parseResult =
              logEntry.ParseFromString(serializedLogEntry.payload);
          !parseResult) {
        throw std::runtime_error{"protobuf parse result"};
      }

      auto result = co_await apply(logEntry);
      lastAppliedLogId_ = nextLogId;
      if (logId == nextLogId) {
        co_return result;
      }
    }

    throw std::runtime_error{"unknown error from virtual log state machine"};
  }

  coro<R> apply(rk::projects::durable_log::LogEntry_1 t) override {
    co_return co_await upstreamStateMachine_->apply(std::move(t));
  }

  coro<void> sync() override { co_return; }

  void setApplicator(
      std::shared_ptr<Applicator<LogEntry_1, R>> applicator) override {}

  void setUpstreamStateMachine(std::shared_ptr<StateMachine<LogEntry_1, R>>
                                   upstreamStateMachine) override {
    upstreamStateMachine_ = std::move(upstreamStateMachine);
  }

private:
  std::shared_ptr<VirtualLog> virtualLog_;
  std::shared_ptr<StateMachine<LogEntry_1, R>> upstreamStateMachine_;
  LogId lastAppliedLogId_;
};

} // namespace rk::projects::state_machine