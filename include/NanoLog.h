//
// Created by Rahul  Kushwaha on 12/29/22.
//

#ifndef LOGSTORAGE_NANOLOG_H
#define LOGSTORAGE_NANOLOG_H

#include "Common.h"
#include "DurableLog.h"

namespace rk::project::counter {

class NanoLog {
 public:
  virtual std::string getId() = 0;
  virtual std::string getName() = 0;
  virtual std::string getMetadataVersionId() = 0;

  virtual LogId
  append(LogId logId, std::string logEntryPayload, bool skipSeal = false) = 0;
  virtual std::variant<LogEntry, LogReadError> getLogEntry(LogId logId) = 0;
  virtual LogId seal() = 0;

  virtual LogId getStartIndex() = 0;
  virtual LogId getEndIndex() = 0;
  virtual bool isSealed() = 0;

  virtual ~NanoLog() {}
};

class NanoLogSealedException: public std::exception {
  static constexpr char *msg{"NanoLog is sealed."};

 public:
  char *what() {
    return msg;
  }
};

}

#endif //LOGSTORAGE_NANOLOG_H
