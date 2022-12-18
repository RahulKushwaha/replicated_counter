//
// Created by Rahul  Kushwaha on 11/24/22.
//

#ifndef LOGSTORAGE_DURABLELOG_H
#define LOGSTORAGE_DURABLELOG_H

#include <string>
#include <variant>

using LogId = std::int64_t;

struct LogEntry {
  LogId logId;
  std::string payload;
};

enum class LogReadError {
  IndexOutOfBounds
};

class DurableLog {
 public:
  virtual std::string getName() = 0;
  virtual LogId append(std::string logEntryPayload) = 0;
  virtual std::variant<LogEntry, LogReadError> getLogEntry(LogId logId) = 0;

  /** All the entries smaller than the provided logId will be trimmed(not
   * including the given logId).
   * */
  virtual void trim(LogId logId) = 0;

  virtual LogId sync() = 0;

  virtual void pauseTrim() = 0;
  virtual void unPauseTrim() = 0;
};


#endif //LOGSTORAGE_DURABLELOG_H
