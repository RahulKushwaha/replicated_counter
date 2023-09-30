//
// Created by Rahul  Kushwaha on 12/29/22.
//

#pragma once

#include "Common.h"
#include "fmt/format.h"
#include "folly/futures/Future.h"

namespace rk::projects::durable_log {

class NanoLog {
 public:
  virtual std::string getId() = 0;
  virtual std::string getName() = 0;
  virtual std::string getMetadataVersionId() = 0;

  virtual coro<LogId> append(std::optional<LogId> globalCommitIndex,
                             LogId logId, std::string logEntryPayload,
                             bool skipSeal = false) = 0;
  virtual coro<std::variant<LogEntry, LogReadError>> getLogEntry(
      LogId logId) = 0;
  virtual coro<LogId> seal() = 0;
  virtual coro<LogId> getLocalCommitIndex() = 0;
  virtual coro<LogId> trim(LogId logId) = 0;

  virtual LogId getStartIndex() = 0;
  virtual LogId getEndIndex() = 0;
  virtual bool isSealed() = 0;

  virtual ~NanoLog() = default;
};

class NanoLogSealedException : public std::logic_error {
 public:
  explicit NanoLogSealedException(std::string versionId)
      : std::logic_error(
            fmt::format("nanolog is sealed. versionId: {}", versionId)) {}
};

class NanoLogLogPositionAlreadyOccupied : public std::exception {
 public:
  const char* what() const noexcept override {
    return "there is already a log entry at that position.";
  }
};

class NanoLogLogNotAvailable : public std::exception {
 public:
  const char* what() const noexcept override {
    return "NanoLog is not available.";
  }
};

}  // namespace rk::projects::durable_log