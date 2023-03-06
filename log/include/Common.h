//
// Created by Rahul  Kushwaha on 12/29/22.
//

#pragma once

#include <cstdint>
#include <string>

namespace rk::projects::durable_log {

using LogId = std::int64_t;

constexpr LogId
    LowestNonExistingLogId = std::numeric_limits<std::int64_t>::min();

constexpr LogId
    HighestNonExistingLogId = std::numeric_limits<std::int64_t>::max();

struct LogEntry {
  LogId logId;
  std::string payload;
};

enum class LogReadError {
  IndexOutOfBounds,
  NotFound,
  Unknown,
};

class NonRecoverableError: public std::exception {
 public:
  const char *what() const _NOEXCEPT override {
    return "NonRecoverableError";
  }
};

class NotImplementedException: public std::exception {
 public:
  const char *what() const _NOEXCEPT override {
    return "Method Not Implmented";
  }
};

}