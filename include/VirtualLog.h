//
// Created by Rahul  Kushwaha on 12/29/22.
//

#ifndef LOGSTORAGE_VIRTUALLOG_H
#define LOGSTORAGE_VIRTUALLOG_H
#include <string>
#include "Common.h"

namespace rk::project::counter {

class VirtualLog {
 public:
  virtual std::string getId() = 0;
  virtual std::string getName() = 0;
  virtual LogId append(std::string logEntryPayload) = 0;
  virtual std::variant<LogEntry, LogReadError> getLogEntry(LogId logId) = 0;
  virtual void reconfigure() = 0;

  virtual ~VirtualLog() {}
};

}

#endif //LOGSTORAGE_VIRTUALLOG_H
