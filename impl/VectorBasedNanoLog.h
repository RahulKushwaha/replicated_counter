//
// Created by Rahul  Kushwaha on 12/31/22.
//

#ifndef LOGSTORAGE_VECTORBASEDNANOLOG_H
#define LOGSTORAGE_VECTORBASEDNANOLOG_H

#include "../include/NanoLog.h"
#include <map>

namespace rk::project::counter {

class VectorBasedNanoLog: public NanoLog {
 public:
  explicit VectorBasedNanoLog(
      std::string id,
      std::string name,
      LogId startIndex,
      LogId endIndex,
      bool sealed);

 public:
  std::string getId() override;
  std::string getName() override;
  LogId
  append(LogId logId,
         std::string logEntryPayload,
         bool skipSeal = false) override;
  std::variant<LogEntry, LogReadError> getLogEntry(LogId logId) override;

  LogId seal() override;

  LogId getStartIndex() override;
  LogId getEndIndex() override;
  bool isSealed() override;

 private:
  std::string id_;
  std::string name_;
  LogId startIndex_;
  LogId endIndex_;
  bool sealed_;
  std::map<LogId, std::string> logs_;
};
}
#endif //LOGSTORAGE_VECTORBASEDNANOLOG_H
