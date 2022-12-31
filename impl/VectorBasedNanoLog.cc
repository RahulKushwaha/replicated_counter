//
// Created by Rahul  Kushwaha on 12/31/22.
//

#include "VectorBasedNanoLog.h"

namespace rk::project::counter {

VectorBasedNanoLog::VectorBasedNanoLog(
    std::string id,
    std::string name,
    LogId startIndex,
    LogId endIndex,
    bool sealed)
    : id_(std::move(id)), name_(std::move(name)), startIndex_(startIndex),
      endIndex_(endIndex),
      sealed_(sealed), logs_{} {}

std::string VectorBasedNanoLog::getId() {
  return id_;
}

std::string VectorBasedNanoLog::getName() {
  return name_;
}

LogId VectorBasedNanoLog::append(LogId logId,
                                 std::string logEntryPayload,
                                 bool skipSeal) {
  if (!skipSeal && sealed_) {
    throw NanoLogSealedException{};
  }

  logs_[logId] = logEntryPayload;
  return logId;
}

std::variant<LogEntry, LogReadError>
VectorBasedNanoLog::getLogEntry(LogId logId) {
  if (auto itr = logs_.find(logId); itr != logs_.end()) {
    return {LogEntry{logId, itr->second}};
  }

  return {LogReadError::NotFound};
}

LogId VectorBasedNanoLog::seal() {
  sealed_ = true;
  return endIndex_;
}

LogId VectorBasedNanoLog::getStartIndex() {
  return startIndex_;
}

LogId VectorBasedNanoLog::getEndIndex() {
  return endIndex_;
}

bool VectorBasedNanoLog::isSealed() {
  return sealed_;
}

}