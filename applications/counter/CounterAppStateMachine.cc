//
// Created by Rahul  Kushwaha on 7/15/23.
//
#include "applications/counter/CounterAppStateMachine.h"
#include "applications/counter/CounterApp.h"

namespace rk::projects::counter_app {

CounterAppStateMachine::CounterAppStateMachine(
    std::shared_ptr<
        state_machine::StateMachine<durable_log::LogEntry_1, ReturnType>>
        downstreamStateMachine)
    : lastAppliedLogId_{0},
      downstreamStateMachine_{std::move(downstreamStateMachine)},
      applicator_{nullptr} {}

folly::coro::Task<std::vector<CounterKeyValue>>
CounterAppStateMachine::append(CounterLogEntries t) {
  durable_log::LogEntry_1 logEntry{};
  durable_log::SingleLogEntry singleLogEntry{};
  singleLogEntry.set_payload(t.SerializeAsString());
  logEntry.mutable_single_log_entry()->CopyFrom(singleLogEntry);

  auto result = co_await append(logEntry);

  if (std::holds_alternative<std::vector<CounterKeyValue>>(result)) {
    co_return std::get<std::vector<CounterKeyValue>>(result);
  }

  throw std::runtime_error{"unknown result type"};
}

folly::coro::Task<ReturnType>
CounterAppStateMachine::append(rk::projects::durable_log::LogEntry_1 t) {
  assert(downstreamStateMachine_ != nullptr);

  co_return co_await downstreamStateMachine_->append(std::move(t));
}

folly::coro::Task<ReturnType>
CounterAppStateMachine::apply(rk::projects::durable_log::LogEntry_1 t) {
  assert(applicator_ != nullptr);

  co_return co_await applicator_->apply(std::move(t));
}

void CounterAppStateMachine::setApplicator(
    std::shared_ptr<applicator_t> applicator) {
  applicator_ = std::move(applicator);
}

void CounterAppStateMachine::setUpstreamStateMachine(
    std::shared_ptr<
        state_machine::StateMachine<durable_log::LogEntry_1, ReturnType>>
        upstreamStateMachine_) {
  throw std::runtime_error{
      "counterapp state machine does not require upstream state machine"};
}

folly::coro::Task<void> CounterAppStateMachine::sync() {
  co_await downstreamStateMachine_->sync();
}

} // namespace rk::projects::counter_app