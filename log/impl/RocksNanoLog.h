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
                        std::shared_ptr<persistence::KVStoreLite> kvStore)
      : id_{std::move(id)}, name_{std::move(name)},
        versionId_{std::move(versionId)}, startIndex_{startIndex},
        endIndexDirty_{true}, endIndex_{endIndex}, sealed_{sealed},
        sealDirty_{true}, completionQueue_{startIndex_},
        kvStore_{std::move(kvStore)} {
    // Remove these sync calls.
    kvStore_
        ->put(fmt::format(formatter.startIndexKey(), versionId_),
              std::to_string(startIndex))
        .semi()
        .get();
    kvStore_
        ->put(fmt::format(formatter.endIndexKey(), versionId_),
              std::to_string(startIndex))
        .semi()
        .get();
    kvStore_->put(fmt::format(formatter.sealKey(), versionId_), "FALSE")
        .semi()
        .get();
  }

  std::string getId() override { return id_; };
  std::string getName() override { return name_; };
  std::string getMetadataVersionId() override { return versionId_; }

  coro<LogId> append(std::optional<LogId> globalCommitIndex, LogId logId,
                     std::string logEntryPayload,
                     bool skipSeal = false) override {
    auto sealed = co_await co_isSealed();
    if (!skipSeal && sealed) {
      throw NanoLogSealedException(versionId_);
    }

    auto logKey = fmt::format(formatter.logKey(), versionId_, logId);
    auto append = co_await kvStore_->putIfNotExists(logKey, logEntryPayload);
    if (!append) {
      throw NanoLogLogPositionAlreadyOccupied();
    }

    auto [promise, future] = folly::makePromiseContract<LogId>();
    if (globalCommitIndex.has_value()) {
      completionQueue_.add(logId, std::move(promise), logId);
      completionQueue_.completeAllBelow(globalCommitIndex.value());
      co_await co_setEndIndex(globalCommitIndex.value() + 1);
    } else {
      completionQueue_.add(logId, std::move(promise), logId, false);
      auto range = completionQueue_.getRangeEligibleForCompletion();
      if (range.second > range.first) {
        co_await co_setEndIndex(range.second);
        completionQueue_.completeAllBelow(range.second - 1);
      }
    }

    co_return co_await std::move(future);
  }

  coro<std::variant<LogEntry, LogReadError>> getLogEntry(LogId logId) override {
    try {
      auto log = co_await kvStore_->get(
          fmt::format(formatter.logKey(), versionId_, logId));

      if (log.has_value()) {
        co_return {LogEntry{.logId = logId, .payload = log.value()}};
      } else {
        co_return {LogReadError::NotFound};
      }
    } catch (...) {
      co_return {LogReadError::Unknown};
    }
  }

  LogId seal() override {
    if (co_setSeal().semi().get()) {
      return endIndex_;
    }

    return -1;
  }

  LogId getLocalCommitIndex() override {
    return co_getEndIndex().semi().get();
  };
  LogId getStartIndex() override { return startIndex_; };
  LogId getEndIndex() override { return co_getEndIndex().semi().get(); };
  bool isSealed() override { return co_isSealed().semi().get(); };

  coro<bool> co_isSealed() {
    if (sealDirty_) {
      auto optionalSealed =
          co_await kvStore_->get(fmt::format(formatter.sealKey(), versionId_));

      if (optionalSealed.has_value()) {
        sealed_ = folly::to<bool>(optionalSealed.value());
      }

      sealDirty_ = false;
    }

    co_return sealed_;
  }

  coro<bool> co_setSeal() {
    sealDirty_ = true;

    auto key = fmt::format(formatter.sealKey(), versionId_);
    auto result = co_await kvStore_->put(key, "TRUE");
    auto fsyncResult = co_await kvStore_->flushWal();

    if (fsyncResult) {
      sealDirty_ = false;
      sealed_ = true;
    }

    co_return result &&fsyncResult &&sealed_;
  }

  coro<LogId> co_getEndIndex() {
    if (endIndexDirty_) {
      auto optionalEndIndex = co_await kvStore_->get(
          fmt::format(formatter.endIndexKey(), versionId_));

      if (optionalEndIndex.has_value()) {
        endIndex_ = folly::to<LogId>(optionalEndIndex.value());
      }

      endIndexDirty_ = false;
    }

    co_return endIndex_;
  }

  coro<bool> co_setEndIndex(LogId endIndex) {
    endIndexDirty_ = true;

    auto key = fmt::format(formatter.endIndexKey(), versionId_);
    auto result = co_await kvStore_->put(key, std::to_string(endIndex));
    auto fsyncResult = co_await kvStore_->flushWal();

    if (fsyncResult) {
      endIndexDirty_ = false;
      endIndex_ = endIndex;
    } else {
      throw std::runtime_error{"unknown error while writing to rocks"};
    }

    co_return true;
  }

  ~RocksNanoLog() override = default;

  struct KeyFormat {
    std::string logKey() { return "VERSION_ID|{}|LOG_ID|{}"; }
    std::string sealKey() { return "VERSION_ID|{}|SEAL|"; }
    std::string startIndexKey() { return "VERSION_ID|{}|START_INDEX|"; }
    std::string endIndexKey() { return "VERSION_ID|{}|END_INDEX|"; }
  };

  KeyFormat formatter;

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
};

} // namespace rk::projects::durable_log
