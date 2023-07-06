//
// Created by Rahul  Kushwaha on 12/31/22.
//

#pragma once

#include "log/include/NanoLog.h"
#include "log/utils/OrderedCompletionQueue.h"
#include <map>

namespace rk::projects::durable_log {

using namespace rk::projects::utils;

class InMemoryNanoLog : public NanoLog {
public:
  explicit InMemoryNanoLog(std::string id, std::string name,
                           std::string metadataVersionId, LogId startIndex,
                           LogId endIndex, bool sealed);

public:
  std::string getId() override;
  std::string getName() override;
  std::string getMetadataVersionId() override;

  coro<LogId> append(std::optional<LogId> globalCommitIndex, LogId logId,
                     std::string logEntryPayload,
                     bool skipSeal = false) override;
  coro<std::variant<LogEntry, LogReadError>> getLogEntry(LogId logId) override;

  coro<LogId> seal() override;
  coro<LogId> getLocalCommitIndex() override;

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
  std::shared_ptr<std::mutex> mtx_;
};
} // namespace rk::projects::durable_log