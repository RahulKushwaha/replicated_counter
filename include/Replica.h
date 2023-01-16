//
// Created by Rahul  Kushwaha on 12/31/22.
//

#ifndef LOGSTORAGE_REPLICA_H
#define LOGSTORAGE_REPLICA_H
#include <string>
#include <variant>
#include <folly/futures/Future.h>

#include "Common.h"
#include "MetadataStore.h"


namespace rk::project::counter {

class Replica {
 public:
  virtual std::string getId() = 0;
  virtual std::string getName() = 0;
  virtual folly::SemiFuture<folly::Unit>
  append(LogId logId,
         std::string logEntryPayload,
         bool skipSeal = false) = 0;
  virtual folly::SemiFuture<std::variant<LogEntry, LogReadError>>
  getLogEntry(LogId logId) = 0;
  virtual LogId seal(VersionId versionId) = 0;
};

class MetadataBlockNotPresent: public std::exception {
 public:
  const char *what() const _NOEXCEPT override {
    return "MetadataBlock containing the log_id is not present.";
  }
};

}

#endif //LOGSTORAGE_REPLICA_H
