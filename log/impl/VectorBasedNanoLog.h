//
// Created by Rahul  Kushwaha on 12/31/22.
//

#pragma once

#include "../include/NanoLog.h"
#include "../utils/OrderedCompletionQueue.h"
#include <map>

namespace rk::projects::durable_log {

using namespace rk::projects::utils;

class VectorBasedNanoLog: public NanoLog {
 public:
  explicit VectorBasedNanoLog(
      std::string id,
      std::string name,
      std::string metadataVersionId,
      LogId startIndex,
      LogId endIndex,
      bool sealed);

 public:
  std::string getId() override;
  std::string getName() override;
  std::string getMetadataVersionId() override;

  folly::SemiFuture<LogId>
  append(LogId logId,
         std::string logEntryPayload,
         bool skipSeal = false) override;
  std::variant<LogEntry, LogReadError> getLogEntry(LogId logId) override;

  LogId seal() override;
  LogId getLocalCommitIndex() override;

  LogId getStartIndex() override;
  LogId getEndIndex() override;
  bool isSealed() override;

 private:
  std::string id_;
  std::string name_;
  std::string metadataVersionId_;
  LogId startIndex_;
  LogId endIndex_;
  bool sealed_;
  std::map<LogId, std::string> logs_;
  OrderedCompletionQueue<LogId> completionQueue_;
};
}