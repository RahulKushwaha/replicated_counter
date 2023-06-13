//
// Created by Rahul  Kushwaha on 2/20/23.
//

#include "CounterApp.h"

namespace rk::projects::counter_app {

CounterApp::CounterApp(std::shared_ptr<VirtualLog> virtualLog)
    : virtualLog_{std::move(virtualLog)}, lastAppliedEntry_{0},
      mtx_{std::make_unique<std::mutex>()}, lookup_{} {}

folly::coro::Task<std::int64_t>
CounterApp::incrementAndGet(std::string key, std::int64_t incrBy) {
  std::lock_guard lk{*mtx_};

  std::vector<Operation> operations;
  Operation op = IncrOperation{key, incrBy};
  operations.emplace_back(op);

  LogId logId = co_await virtualLog_->append(serialize(operations));

  sync(logId);

  co_return lookup_[key];
}

folly::coro::Task<std::int64_t>
CounterApp::decrementAndGet(std::string key, std::int64_t decrBy) {
  std::lock_guard lk{*mtx_};
  std::vector<Operation> operations;
  Operation op = DecrOperation{key, decrBy};
  operations.emplace_back(op);

  LogId logId = co_await virtualLog_->append(serialize(operations));
  sync(logId);

  co_return lookup_[key];
}

folly::coro::Task<std::int64_t> CounterApp::getValue(std::string key) {
  std::lock_guard lk{*mtx_};
  auto latestLogId = co_await virtualLog_->sync();
  sync(latestLogId);

  co_return lookup_[key];
}

folly::coro::Task<std::vector<CounterApp::CounterValue>>
CounterApp::batchUptate(std::vector<Operation> operations) {
  std::lock_guard lk{*mtx_};
  std::vector<std::int64_t> result;

  LogId logId = co_await virtualLog_->append(serialize(operations));

  std::vector<CounterApp::CounterValue> counterValues = sync(logId);

  co_return counterValues;
}

std::vector<CounterApp::CounterValue> CounterApp::sync(LogId to) {
  LogId logIdToApply = lastAppliedEntry_ + 1;
  std::vector<CounterApp::CounterValue> counterValues;
  while (logIdToApply <= to) {
    if (logIdToApply == to) {
      counterValues = applyLogEntries(logIdToApply);

    } else {
      applyLogEntries(logIdToApply);
    }
    lastAppliedEntry_ = logIdToApply++;
  }

  return counterValues;
}

std::vector<CounterApp::CounterValue>
CounterApp::applyLogEntries(LogId logIdToApply) {
  auto logEntryResponse = virtualLog_->getLogEntry(logIdToApply).get();
  if (!std::holds_alternative<LogEntry>(logEntryResponse)) {
    throw std::runtime_error{"Non Recoverable Error[Found unknown entry]. The "
                             "application must crash."};
  }

  auto logEntrySerialized = std::get<LogEntry>(logEntryResponse);
  auto logEnteries = deserialize(logEntrySerialized.payload);
  return apply(logEnteries);
}

/* static */ std::string
CounterApp::serialize(std::string key, std::int64_t val,
                      CounterLogEntry_CommandType commandType) {
  CounterLogEntry entry;
  entry.set_commandtype(commandType);
  entry.set_key(std::move(key));
  entry.set_val(val);

  return entry.SerializeAsString();
}

std::string CounterApp::serialize(const std::vector<Operation> &operations) {

  CounterLogEnteries enteries;

  for (auto op : operations) {

    if (std::holds_alternative<CounterApp::IncrOperation>(op)) {
      CounterLogEntry entry;
      entry.set_commandtype(CounterLogEntry_CommandType_INCR);
      CounterApp::IncrOperation incrOperation =
          std::get<CounterApp::IncrOperation>(op);
      entry.set_key(std::move(incrOperation.key));
      entry.set_val(incrOperation.incrBy);
      enteries.mutable_enteries()->Add(std::move(entry));
    } else if (std::holds_alternative<CounterApp::DecrOperation>(op)) {
      CounterLogEntry entry;
      entry.set_commandtype(CounterLogEntry_CommandType_DECR);
      CounterApp::DecrOperation decrOperation =
          std::get<CounterApp::DecrOperation>(op);
      entry.set_key(std::move(decrOperation.key));
      entry.set_val(decrOperation.decrBy);
      enteries.mutable_enteries()->Add(std::move(entry));
    }
  }
  return enteries.SerializeAsString();
}

/* static */ CounterLogEnteries
CounterApp::deserialize(const std::string &payload) {
  CounterLogEnteries enteries;
  enteries.ParseFromString(payload);
  return enteries;
}

std::vector<CounterApp::CounterValue>
CounterApp::apply(const CounterLogEnteries &counterLogEnteries) {

  std::vector<CounterApp::CounterValue> counterValues;
  for (const auto &counterLogEntry : counterLogEnteries.enteries()) {
    auto &val = lookup_[counterLogEntry.key()];

    CounterValue counterValue;

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
