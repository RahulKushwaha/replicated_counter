//
// Created by Rahul  Kushwaha on 12/29/22.
//

#ifndef LOGSTORAGE_VIRTUALLOG_H
#define LOGSTORAGE_VIRTUALLOG_H
#include <string>
#include <folly/futures/Future.h>
#include "Common.h"

namespace rk::project::counter {

class VirtualLog {
 public:
  virtual std::string getId() = 0;
  virtual std::string getName() = 0;
  virtual folly::SemiFuture<LogId> append(std::string logEntryPayload) = 0;
  virtual folly::SemiFuture<std::variant<LogEntry, LogReadError>>
  getLogEntry(LogId logId) = 0;
  virtual void reconfigure() = 0;

  virtual ~VirtualLog() = default;
};

}

#endif //LOGSTORAGE_VIRTUALLOG_H
