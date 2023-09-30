//
// Created by Rahul  Kushwaha on 7/17/23.
//
#pragma once
#include "statemachine/LogTrimApplicator.h"
#include "statemachine/LogTrimStateMachine.h"
#include "statemachine/VirtualLogStateMachine.h"
#include "statemachine/include/StateMachine.h"

namespace rk::projects::state_machine {

template <typename R>
std::shared_ptr<StateMachine<LogEntry_1, R>> makeStateMachineStack(
    std::shared_ptr<persistence::KVStoreLite> kvStore,
    std::shared_ptr<VirtualLog> virtualLog) {
  auto virtualLogStateMachine =
      std::make_shared<VirtualLogStateMachine<R>>(std::move(virtualLog));

  auto logTrimApplicator =
      std::make_shared<LogTrimApplicator<R>>(std::move(kvStore));

  auto logTrimStateMachine = std::make_shared<LogTrimStateMachine<R>>(
      logTrimApplicator, virtualLogStateMachine);

  virtualLogStateMachine->setUpstreamStateMachine(logTrimStateMachine);

  return logTrimStateMachine;
}

}  // namespace rk::projects::state_machine
