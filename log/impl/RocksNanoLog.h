//
// Created by Rahul  Kushwaha on 7/2/23.
//

#pragma once
#include "folly/Conv.h"
#include "log/include/NanoLog.h"
#include "log/utils/OrderedCompletionQueue.h"
#include "persistence/KVStoreLite.h"

namespace rk::projects::durable_log {

class RocksNanoLog : public NanoLog {
public:
  explicit RocksNanoLog(std::string id, std::string name, std::string versionId,
                        LogId startIndex, LogId endIndex, bool sealed,
                        std::shared_ptr<persistence::KVStoreLite> kvStore);

  coro<void> init();

  std::string getId() override;
  std::string getName() override;
  std::string getMetadataVersionId() override;

  coro<LogId> append(std::optional<LogId> globalCommitIndex, LogId logId,
                     std::string logEntryPayload,
                     bool skipSeal = false) override;
  coro<std::variant<LogEntry, LogReadError>> getLogEntry(LogId logId) override;
  coro<LogId> seal() override;
  coro<LogId> getLocalCommitIndex() override;
  coro<LogId> trim(LogId logId) override;

  LogId getStartIndex() override;
  LogId getEndIndex() override;
  bool isSealed() override;
  ~RocksNanoLog() override = default;

private:
  coro<bool> co_isSealed();
  coro<bool> co_setSeal();
  coro<LogId> co_getEndIndex();
  coro<bool> co_setEndIndex(LogId endIndex);

  struct KeyFormat {
    static constexpr std::string_view logKey = "VERSION_ID|{}|LOG_ID|{}";
    static constexpr std::string_view sealKey = "VERSION_ID|{}|SEAL|";
    static constexpr std::string_view startIndexKey =
        "VERSION_ID|{}|START_INDEX|";
    static constexpr std::string_view endIndexKey = "VERSION_ID|{}|END_INDEX|";
  };

private:
  std::string id_;
  std::string name_;
  std::string versionId_;
  LogId startIndex_;
  LogId endIndex_;
  LogId endIndexDirty_;
  bool sealed_;
  bool sealDirty_;
  utils::OrderedCompletionQueue<LogId> completionQueue_;
  std::shared_ptr<persistence::KVStoreLite> kvStore_;
  std::unique_ptr<std::mutex> mtx_;
};

} // namespace rk::projects::durable_log
