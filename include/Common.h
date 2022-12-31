//
// Created by Rahul  Kushwaha on 12/29/22.
//

#ifndef LOGSTORAGE_COMMON_H
#define LOGSTORAGE_COMMON_H
#include <cstdint>
#include <string>

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
  NotFound
};


#endif //LOGSTORAGE_COMMON_H
