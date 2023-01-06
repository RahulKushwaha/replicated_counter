//
// Created by Rahul  Kushwaha on 12/31/22.
//

#ifndef LOGSTORAGE_REPLICA_H
#define LOGSTORAGE_REPLICA_H
#include <string>
#include "Common.h"
#include <folly/futures/Future.h>

namespace rk::project::counter {

class Replica {
 public:
  virtual std::string getId() = 0;
  virtual std::string getName() = 0;
  virtual folly::SemiFuture<folly::Unit>
  append(LogId logId, std::string logEntryPayload) = 0;
  virtual folly::SemiFuture<std::variant<LogEntry, LogReadError>>
  getLogEntry(LogId logId) = 0;
  virtual LogId seal() = 0;
};

}

#endif //LOGSTORAGE_REPLICA_H
