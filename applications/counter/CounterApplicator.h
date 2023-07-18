//
// Created by Rahul  Kushwaha on 7/15/23.
//

#pragma once
#include "applications/counter/proto/CounterEntry.pb.h"
#include "folly/experimental/coro/Task.h"
#include "log/proto/LogEntry.pb.h"
#include "statemachine/include/StateMachine.h"

namespace rk::projects::counter_app {

class CounterApp;
class CounterKeyValue;

using applicatorInput_t = durable_log::LogEntry_1;
using applicatorOutput_t =
    state_machine::ReturnType<std::vector<CounterKeyValue>, folly::Unit>;

class CounterApplicator
    : public state_machine::Applicator<applicatorInput_t, applicatorOutput_t> {

public:
  explicit CounterApplicator(std::shared_ptr<CounterApp> app);

  folly::coro::Task<applicatorOutput_t> apply(applicatorInput_t t) override;

private:
  std::shared_ptr<CounterApp> app_;
};
} // namespace rk::projects::counter_app