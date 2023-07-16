//
// Created by Rahul  Kushwaha on 7/15/23.
//
#pragma once
#include "applications/counter/CounterApplicator.h"
#include "applications/counter/proto/CounterEntry.pb.h"
#include "applications/counter/server/proto/CounterService.pb.h"
#include "log/include/VirtualLog.h"
#include "statemachine/include/StateMachine.h"

namespace rk::projects::counter_app {

class CounterApp;
class CounterKeyValue;

class CounterAppStateMachine
    : public state_machine::StateMachine<CounterLogEnteries,
                                         std::vector<CounterKeyValue>> {
  using applicator_t = state_machine::Applicator<CounterLogEnteries,
                                                 std::vector<CounterKeyValue>>;

public:
  explicit CounterAppStateMachine(
      std::shared_ptr<durable_log::VirtualLog> virtualLog);

  folly::coro::Task<std::vector<CounterKeyValue>>
  append(CounterLogEnteries t) override;
  void setApplicator(std::shared_ptr<applicator_t> applicator) override;
  folly::coro::Task<void> sync() override;

private:
  durable_log::LogId lastAppliedLogId_;
  std::shared_ptr<durable_log::VirtualLog> virtualLog_;
  std::shared_ptr<applicator_t> applicator_;
};

} // namespace rk::projects::counter_app
