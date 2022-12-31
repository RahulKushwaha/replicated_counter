//
// Created by Rahul  Kushwaha on 11/24/22.
//

#include <iostream>
#include "VectorBasedDurableLog.h"

VectorBasedDurableLog::VectorBasedDurableLog(
    std::string name,
    LogId startIndex) : name_{std::move(name)},
                        startIndex_{startIndex},
                        endIndex_{startIndex},
                        trimPaused_{false} {}

std::string VectorBasedDurableLog::getName() {
  return name_;
}

LogId VectorBasedDurableLog::append(std::string logEntryPayload) {
  LogId nextLogId = endIndex_++;
  logs_.emplace_back(std::move(logEntryPayload));

  return nextLogId;
}

std::variant<LogEntry, LogReadError>
VectorBasedDurableLog::getLogEntry(LogId logId) {
  if (logId < startIndex_ || logId >= endIndex_) {
    return {LogReadError::IndexOutOfBounds};
  }

  return {LogEntry{logId, logs_.at(logId)}};
}

LogId VectorBasedDurableLog::sync() {
  return endIndex_ - 1;
}