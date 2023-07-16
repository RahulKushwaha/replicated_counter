//
// Created by Rahul  Kushwaha on 7/15/23.
//
#include "applications/counter/CounterAppStateMachine.h"
#include "applications/counter/CounterApp.h"

namespace rk::projects::counter_app {

CounterAppStateMachine::CounterAppStateMachine(
    std::shared_ptr<durable_log::VirtualLog> virtualLog)
    : lastAppliedLogId_{0}, virtualLog_{std::move(virtualLog)},
      applicator_{nullptr} {}

folly::coro::Task<std::vector<CounterKeyValue>>
CounterAppStateMachine::append(CounterLogEnteries t) {
  auto logId = co_await virtualLog_->append(t.SerializeAsString());
  while (lastAppliedLogId_ <= logId) {
    auto nextLog = co_await virtualLog_->getLogEntry(logId);
    if (std::holds_alternative<durable_log::LogReadError>(nextLog)) {
      auto logReadError = std::get<durable_log::LogReadError>(nextLog);
      throw std::runtime_error{"log read error"};
    }

    auto logEntry = std::get<durable_log::LogEntry>(nextLog);
    CounterLogEnteries entries{};
    auto parseResult = entries.ParseFromString(logEntry.payload);
    if (!parseResult) {
      throw std::runtime_error{"protobuf parse result"};
    }

    auto applyResult = co_await applicator_->apply(entries);
    lastAppliedLogId_ = logEntry.logId;
    if (lastAppliedLogId_ == logId) {
      co_return applyResult;
    }
  }

  co_return co_await folly::coro::makeErrorTask<std::vector<CounterKeyValue>>(
      folly::exception_wrapper{std::runtime_error{
          "reached end of loop. should not happen. un-recoverable error."}});
}

void CounterAppStateMachine::setApplicator(
    std::shared_ptr<applicator_t> applicator) {
  applicator_ = std::move(applicator);
}

folly::coro::Task<void> CounterAppStateMachine::sync() {
  co_await virtualLog_->sync();
}

} // namespace rk::projects::counter_app