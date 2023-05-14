//
// Created by Rahul  Kushwaha on 12/29/22.
//

#pragma once

#include <cstdint>
#include <string>

namespace rk::projects::durable_log {

using LogId = std::int64_t;
using VersionId = std::int64_t;

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

inline std::ostream &operator<<(std::ostream &os, LogReadError &readError) {
  switch (readError) {
    case LogReadError::IndexOutOfBounds:
      os << std::string_view{"LogReadError"};
      break;
    case LogReadError::NotFound:
      os << std::string_view{"NotFound"};
      break;
    case LogReadError::Unknown:
      os << std::string_view{"Unknown"};
      break;
  }

  return os;
}

class NonRecoverableError: public std::exception {
 public:
  const char *what() const _NOEXCEPT override {
    return "NonRecoverableError";
  }
};

class NullSequencerException: public std::exception {
 public:
  const char *what() const _NOEXCEPT override {
    return "Null Sequencer cannot answer.";
  }
};

class NotImplementedException: public std::exception {
 public:
  const char *what() const _NOEXCEPT override {
    return "Method Not Implmented";
  }
};

class MetadataBlockNotFound: public std::exception {
 public:
  const char *what() const _NOEXCEPT override {
    return "metadatablock not found";
  }
};

}