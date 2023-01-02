//
// Created by Rahul  Kushwaha on 11/24/22.
//

#ifndef LOGSTORAGE_VECTORBASEDDURABLELOG_H
#define LOGSTORAGE_VECTORBASEDDURABLELOG_H

#include "include/DurableLog.h"
#include <string>
#include <vector>

class VectorBasedDurableLog: public DurableLog {
 public:
  std::string getName() override;
  LogId append(std::string logEntryPayload) override;
  std::variant<LogEntry, LogReadError> getLogEntry(LogId logId) override;
  void append(LogId logId, std::string logEntryPayload) override {}

  LogId sync() override;

  VectorBasedDurableLog(std::string name, LogId startIndex);
 private:
  std::string name_;
  std::vector<std::string> logs_;
  LogId startIndex_;

  // Points to the next empty location.
  LogId endIndex_;
  std::atomic_bool trimPaused_;
};


#endif //LOGSTORAGE_VECTORBASEDDURABLELOG_H
