//
// Created by Rahul  Kushwaha on 11/24/22.
//
#include <variant>
#include <iostream>
#include "Counter.h"

namespace rk::project::counter {

Counter::Counter(long val, std::shared_ptr<DurableLog> durableLog,
                 LogId lastAppliedPosition)
    : val_{val}, log_{std::move(durableLog)},
      lastAppliedPosition_{lastAppliedPosition} {
}

void Counter::apply(const CounterLogEntry &counterLogEntry) {
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

std::int64_t Counter::get() {
  auto finalLogId = log_->sync();
  auto init = lastAppliedPosition_;

  while (finalLogId >= init + 1) {
    init++;
    auto logEntryResponse = log_->getLogEntry(init);
    if (!std::holds_alternative<LogEntry>(logEntryResponse)) {
      throw std::runtime_error{
          "Non Recoverable Error[Found unknown entry]. The application must crash."};
    }

    std::cout << "Successfully Applied: " << init << std::endl;
    CounterLogEntry logEntry;
    bool result =
        logEntry.ParseFromString(std::get<LogEntry>(logEntryResponse).payload);

    if (!result) {
      throw std::runtime_error{
          "Non Recoverable Error[Protobuf DeSer Failure]. The application must crash."};
    }

    apply(logEntry);
  }

  lastAppliedPosition_ = finalLogId;

  return val_.load();
}

}