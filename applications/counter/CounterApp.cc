//
// Created by Rahul  Kushwaha on 2/20/23.
//

#include "CounterApp.h"

namespace rk::projects::counter_app {

CounterApp::CounterApp(std::shared_ptr<CounterAppStateMachine> stateMachine)
    : stateMachine_{std::move(stateMachine)}, lastAppliedEntry_{0},
      lastSnapshotId_{0}, mtx_{std::make_unique<std::mutex>()}, lookup_{} {}

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
  co_return co_await stateMachine_->append(serialize(operations));
}

CounterLogEntries
CounterApp::serialize(const std::vector<Operation> &operations) {
  CounterLogEntries entries{};

  for (auto op : operations) {
    if (std::holds_alternative<CounterApp::IncrOperation>(op)) {
      CounterLogEntry entry;
      entry.set_commandtype(CounterLogEntry_CommandType_INCR);
      CounterApp::IncrOperation incrOperation =
          std::get<CounterApp::IncrOperation>(op);
      entry.set_key(std::move(incrOperation.key));
      entry.set_val(incrOperation.incrBy);
      entries.mutable_entries()->Add(std::move(entry));
    } else if (std::holds_alternative<CounterApp::DecrOperation>(op)) {
      CounterLogEntry entry;
      entry.set_commandtype(CounterLogEntry_CommandType_DECR);
      CounterApp::DecrOperation decrOperation =
          std::get<CounterApp::DecrOperation>(op);
      entry.set_key(std::move(decrOperation.key));
      entry.set_val(decrOperation.decrBy);
      entries.mutable_entries()->Add(std::move(entry));
    }
  }

  return entries;
}

std::vector<CounterKeyValue>
CounterApp::apply(const CounterLogEntries &counterLogEntries) {
  std::vector<CounterKeyValue> counterValues;
  for (const auto &counterLogEntry : counterLogEntries.entries()) {
    auto &val = lookup_[counterLogEntry.key()];

    CounterKeyValue counterValue;

    if (counterLogEntry.commandtype() ==
        CounterLogEntry_CommandType::CounterLogEntry_CommandType_INCR) {
      val = val + counterLogEntry.val();
      counterValue.key = counterLogEntry.key();
      counterValue.val = val;
      counterValues.emplace_back(std::move(counterValue));

    } else if (counterLogEntry.commandtype() ==
               CounterLogEntry_CommandType::CounterLogEntry_CommandType_DECR) {
      val = val - counterLogEntry.val();
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

coro<CounterAppSnapshot> CounterApp::snapshot() {
  std::lock_guard lock{*mtx_};
  CounterAppSnapshot counterAppSnapshot{};
  counterAppSnapshot.set_log_id(lastAppliedEntry_);
  counterAppSnapshot.mutable_values()->insert(lookup_.begin(), lookup_.end());
  auto result = co_await kvStore_->put("COUNTER_APP_SNAPSHOT",
                                       counterAppSnapshot.SerializeAsString());
  if (result) {
    auto fsyncResult = co_await kvStore_->flushWal();
    if (fsyncResult) {
      lastSnapshotId_ = lastAppliedEntry_;
      co_return counterAppSnapshot;
    }
  }

  throw std::runtime_error{"could not save snapshot"};
}

LogId CounterApp::getLastSnapshotId() const { return lastSnapshotId_; }

} // namespace rk::projects::counter_app
