//
// Created by Rahul  Kushwaha on 2/20/23.
//

#include "CounterApp.h"

namespace rk::projects::counter_app {

CounterApp::CounterApp(std::shared_ptr<VirtualLog> virtualLog)
    : virtualLog_{std::move(virtualLog)}, lastAppliedEntry_{0}, val_{0} {}

std::int64_t CounterApp::incrementAndGet(std::int64_t incrBy) {
  LogId logId = virtualLog_->append(
          serialize(incrBy,
                    CounterLogEntry_CommandType::CounterLogEntry_CommandType_INCR))
      .get();
  sync(logId);

  return val_;
}

std::int64_t CounterApp::decrementAndGet(std::int64_t decrBy) {
  LogId logId = virtualLog_->append(
          serialize(decrBy,
                    CounterLogEntry_CommandType::CounterLogEntry_CommandType_DECR))
      .get();
  sync(logId);

  return val_;
}

std::int64_t CounterApp::get() {
  auto latestLogId = virtualLog_->sync().get();
  sync(latestLogId);
  return val_;
}

void CounterApp::sync(LogId to) {
  LogId logIdToApply = lastAppliedEntry_ + 1;
  while (logIdToApply <= to) {
    auto logEntryResponse = virtualLog_->getLogEntry(logIdToApply).get();
    if (!std::holds_alternative<LogEntry>(logEntryResponse)) {
      throw std::runtime_error{
          "Non Recoverable Error[Found unknown entry]. The application must crash."};
    }

    auto logEntry = std::get<LogEntry>(logEntryResponse);
    auto counterLogEntry = deserialize(logEntry.payload);
    apply(counterLogEntry);

    lastAppliedEntry_ = logIdToApply++;
  }
}

/* static */ std::string CounterApp::serialize(std::int64_t val,
                                               CounterLogEntry_CommandType commandType) {
  CounterLogEntry entry;
  entry.set_commandtype(commandType);
  entry.set_val(val);

  return entry.SerializeAsString();
}

/* static */  CounterLogEntry CounterApp::deserialize(std::string payload) {
  CounterLogEntry entry;
  entry.ParseFromString(payload);
  return entry;
}

void CounterApp::apply(const CounterLogEntry &counterLogEntry) {
  if (counterLogEntry.commandtype()
      == CounterLogEntry_CommandType::CounterLogEntry_CommandType_INCR) {
    val_.store(val_ + counterLogEntry.val());
  } else if (counterLogEntry.commandtype()
      == CounterLogEntry_CommandType::CounterLogEntry_CommandType_DECR) {
    val_.store(val_ - counterLogEntry.val());
  } else {
    throw std::runtime_error{
        "Unknown Command. This is a non-recoverable error."};
  }
}

}
