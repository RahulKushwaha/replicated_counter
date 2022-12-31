//
// Created by Rahul  Kushwaha on 11/24/22.
//

#ifndef LOGSTORAGE_DURABLELOG_H
#define LOGSTORAGE_DURABLELOG_H

#include <string>
#include <variant>
#include "Common.h"

class DurableLog {
 public:
  virtual std::string getName() = 0;
  virtual LogId append(std::string logEntryPayload) = 0;
  virtual void append(LogId logId, std::string logEntryPayload) = 0;
  virtual std::variant<LogEntry, LogReadError> getLogEntry(LogId logId) = 0;

  virtual LogId sync() = 0;

  virtual ~DurableLog() {}
};


#endif //LOGSTORAGE_DURABLELOG_H
