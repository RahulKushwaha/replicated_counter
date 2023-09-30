//
// Created by Rahul  Kushwaha on 9/5/23.
//
#pragma once

#include "log/proto/LogEntry.pb.h"
#include "statemachine/include/StateMachine.h"

namespace rk::projects::state_machine {
using namespace rk::projects::durable_log;

class NullStateMachineError : std::runtime_error {
 public:
  explicit NullStateMachineError(const std::string& errorMessage)
      : std::runtime_error(errorMessage) {}
};

template <typename T, typename R>
class NullStateMachine : public StateMachine<T, R> {

  coro<R> append(T t) {
    throw NullStateMachineError{"NullStateMachine does not work"};
  }

  coro<R> apply(T t) {
    throw NullStateMachineError{"NullStateMachine does not work"};
  }

  coro<void> sync() {
    throw NullStateMachineError{"NullStateMachine does not work"};
  }

  void setApplicator(std::shared_ptr<Applicator<T, R>> applicator) {
    throw NullStateMachineError{"NullStateMachine does not work"};
  }

  virtual void setUpstreamStateMachine(
      std::shared_ptr<StateMachine<T, R>> upstreamStateMachine) {
    throw NullStateMachineError{"NullStateMachine does not work"};
  }
};

}  // namespace rk::projects::state_machine