//
// Created by Rahul  Kushwaha on 2/20/23.
//

#include "CounterApp.h"

namespace rk::projects::counter_app {

CounterApp::CounterApp(std::shared_ptr<CounterAppStateMachine> stateMachine)
    : stateMachine_{std::move(stateMachine)}, lastAppliedEntry_{0},
      mtx_{std::make_unique<std::mutex>()}, lookup_{} {}

folly::coro::Task<std::int64_t>
CounterApp::incrementAndGet(std::string key, std::int64_t incrBy) {
  std::lock_guard lk{*mtx_};

  std::vector<Operation> operations;
  Operation op = IncrOperation{key, incrBy};
  operations.emplace_back(op);

  auto result = co_await stateMachine_->append(serialize(operations));
  co_return result.at(0).val;
}

folly::coro::Task<std::int64_t>
CounterApp::decrementAndGet(std::string key, std::int64_t decrBy) {
  std::lock_guard lk{*mtx_};
  std::vector<Operation> operations;
  Operation op = DecrOperation{key, decrBy};
  operations.emplace_back(op);

  auto result = co_await stateMachine_->append(serialize(operations));
  co_return result.at(0).val;
}

folly::coro::Task<std::int64_t> CounterApp::getValue(std::string key) {
  std::lock_guard lk{*mtx_};
  co_await stateMachine_->sync();
  co_return lookup_[key];
}

folly::coro::Task<std::vector<CounterKeyValue>>
CounterApp::batchUpdate(std::vector<Operation> operations) {
  std::lock_guard lk{*mtx_};
  std::vector<std::int64_t> result;

  std::vector<CounterKeyValue> counterValues =
      co_await stateMachine_->append(serialize(operations));

  co_return counterValues;
}

CounterLogEnteries
CounterApp::serialize(const std::vector<Operation> &operations) {
  CounterLogEnteries entries{};

  for (auto op : operations) {
    if (std::holds_alternative<CounterApp::IncrOperation>(op)) {
      CounterLogEntry entry;
      entry.set_commandtype(CounterLogEntry_CommandType_INCR);
      CounterApp::IncrOperation incrOperation =
          std::get<CounterApp::IncrOperation>(op);
      entry.set_key(std::move(incrOperation.key));
      entry.set_val(incrOperation.incrBy);
      entries.mutable_enteries()->Add(std::move(entry));
    } else if (std::holds_alternative<CounterApp::DecrOperation>(op)) {
      CounterLogEntry entry;
      entry.set_commandtype(CounterLogEntry_CommandType_DECR);
      CounterApp::DecrOperation decrOperation =
          std::get<CounterApp::DecrOperation>(op);
      entry.set_key(std::move(decrOperation.key));
      entry.set_val(decrOperation.decrBy);
      entries.mutable_enteries()->Add(std::move(entry));
    }
  }

  return entries;
}

std::vector<CounterKeyValue>
CounterApp::apply(const CounterLogEnteries &counterLogEntries) {

  std::vector<CounterKeyValue> counterValues;
  for (const auto &counterLogEntry : counterLogEntries.enteries()) {
    auto &val = lookup_[counterLogEntry.key()];

    CounterKeyValue counterValue;

    if (counterLogEntry.commandtype() ==
        CounterLogEntry_CommandType::CounterLogEntry_CommandType_INCR) {
      val.store(val + counterLogEntry.val());
      counterValue.key = counterLogEntry.key();
      counterValue.val = val;
      counterValues.emplace_back(std::move(counterValue));

    } else if (counterLogEntry.commandtype() ==
               CounterLogEntry_CommandType::CounterLogEntry_CommandType_DECR) {
      val.store(val - counterLogEntry.val());
      counterValue.key = counterLogEntry.key();
      counterValue.val = val;
      counterValues.emplace_back(std::move(counterValue));
    } else {
      throw std::runtime_error{
          "Unknown Command. This is a non-recoverable error."};
    }
  }
  return counterValues;
}

} // namespace rk::projects::counter_app
