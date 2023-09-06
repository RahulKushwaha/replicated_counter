//
// Created by Rahul  Kushwaha on 7/15/23.
//
#pragma once
#include "applications/counter/CounterApplicator.h"
#include "applications/counter/proto/CounterEntry.pb.h"
#include "applications/counter/server/proto/CounterService.pb.h"
#include "log/include/VirtualLog.h"
#include "log/proto/LogEntry.pb.h"
#include "statemachine/include/StateMachine.h"

namespace rk::projects::counter_app {

class CounterApp;
class CounterKeyValue;

using ReturnType =
    state_machine::ReturnType<std::vector<CounterKeyValue>, folly::Unit>;

class CounterAppStateMachine
    : public state_machine::StateMachine<durable_log::LogEntry_1, ReturnType> {
  using applicator_t =
      state_machine::Applicator<durable_log::LogEntry_1, ReturnType>;

public:
  explicit CounterAppStateMachine(
      std::shared_ptr<
          state_machine::StateMachine<durable_log::LogEntry_1, ReturnType>>
          downstreamStateMachine);

  folly::coro::Task<std::vector<CounterKeyValue>> append(CounterLogEntries t);

  folly::coro::Task<ReturnType>
  append(rk::projects::durable_log::LogEntry_1 t) override;

  folly::coro::Task<ReturnType>
  apply(rk::projects::durable_log::LogEntry_1 t) override;

  void setApplicator(std::shared_ptr<applicator_t> applicator) override;
  folly::coro::Task<void> sync() override;

  void setUpstreamStateMachine(
      std::shared_ptr<
          state_machine::StateMachine<durable_log::LogEntry_1, ReturnType>>
          upstreamStateMachine_) override;

private:
  durable_log::LogId lastAppliedLogId_;
  std::shared_ptr<
      state_machine::StateMachine<durable_log::LogEntry_1, ReturnType>>
      downstreamStateMachine_;
  std::shared_ptr<applicator_t> applicator_;
};

} // namespace rk::projects::counter_app
