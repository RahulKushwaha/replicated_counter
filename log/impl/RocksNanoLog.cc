//
// Created by Rahul  Kushwaha on 7/2/23.
//

#include "RocksNanoLog.h"
#include "folly/Conv.h"
#include "log/include/NanoLog.h"
#include "log/utils/OrderedCompletionQueue.h"

namespace rk::projects::durable_log {

RocksNanoLog::RocksNanoLog(std::string id, std::string name,
                           std::string versionId, LogId startIndex,
                           LogId endIndex, bool sealed,
                           std::shared_ptr<persistence::KVStoreLite> kvStore)
    : id_{std::move(id)}, name_{std::move(name)},
      versionId_{std::move(versionId)}, startIndex_{startIndex},
      endIndexDirty_{true}, endIndex_{endIndex}, sealed_{sealed},
      sealDirty_{true}, completionQueue_{startIndex_},
      kvStore_{std::move(kvStore)}, mtx_{std::make_unique<std::mutex>()} {}

coro<void> RocksNanoLog::init() {
  co_await kvStore_->put(fmt::format(KeyFormat::startIndexKey, versionId_),
                         std::to_string(startIndex_));
  co_await kvStore_->put(fmt::format(KeyFormat::endIndexKey, versionId_),
                         std::to_string(startIndex_));
  co_await kvStore_->put(fmt::format(KeyFormat::sealKey, versionId_),
                         sealed_ ? "TRUE" : "FALSE");
  co_return;
}

std::string RocksNanoLog::getId() { return id_; };
std::string RocksNanoLog::getName() { return name_; };
std::string RocksNanoLog::getMetadataVersionId() { return versionId_; }

coro<LogId> RocksNanoLog::append(std::optional<LogId> globalCommitIndex,
                                 LogId logId, std::string logEntryPayload,
                                 bool skipSeal) {
  auto sealed = co_await co_isSealed();
  if (!skipSeal && sealed) {
    throw NanoLogSealedException(versionId_);
  }

  auto logKey = fmt::format(KeyFormat::logKey, versionId_, logId);
  auto append = co_await kvStore_->putIfNotExists(logKey, logEntryPayload);
  if (!append) {
    throw NanoLogLogPositionAlreadyOccupied();
  }

  auto [promise, future] = folly::makePromiseContract<LogId>();
  {
    std::lock_guard lk{*mtx_};
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
  }

  co_return co_await std::move(future);
}

coro<std::variant<LogEntry, LogReadError>>
RocksNanoLog::getLogEntry(LogId logId) {
  try {
    auto log = co_await kvStore_->get(
        fmt::format(KeyFormat::logKey, versionId_, logId));

    if (log.has_value()) {
      co_return {LogEntry{.logId = logId, .payload = log.value()}};
    } else {
      co_return {LogReadError::NotFound};
    }
  } catch (...) {
    co_return {LogReadError::Unknown};
  }
}

coro<LogId> RocksNanoLog::seal() {
  if (co_await co_setSeal()) {
    co_return endIndex_;
  }

  co_return -1;
}

coro<LogId> RocksNanoLog::getLocalCommitIndex() {
  co_return co_await co_getEndIndex();
};

LogId RocksNanoLog::getStartIndex() { return startIndex_; };
LogId RocksNanoLog::getEndIndex() { return co_getEndIndex().semi().get(); };
bool RocksNanoLog::isSealed() { return co_isSealed().semi().get(); };

coro<bool> RocksNanoLog::co_isSealed() {
  if (sealDirty_) {
    auto optionalSealed =
        co_await kvStore_->get(fmt::format(KeyFormat::sealKey, versionId_));

    if (optionalSealed.has_value()) {
      sealed_ = folly::to<bool>(optionalSealed.value());
    }

    sealDirty_ = false;
  }

  co_return sealed_;
}

coro<bool> RocksNanoLog::co_setSeal() {
  sealDirty_ = true;

  auto key = fmt::format(KeyFormat::sealKey, versionId_);
  auto result = co_await kvStore_->put(key, "TRUE");
  auto fsyncResult = co_await kvStore_->flushWal();

  if (fsyncResult) {
    sealDirty_ = false;
    sealed_ = true;
  }

  co_return result &&fsyncResult &&sealed_;
}

coro<LogId> RocksNanoLog::co_getEndIndex() {
  if (endIndexDirty_) {
    auto optionalEndIndex =
        co_await kvStore_->get(fmt::format(KeyFormat::endIndexKey, versionId_));

    if (optionalEndIndex.has_value()) {
      endIndex_ = folly::to<LogId>(optionalEndIndex.value());
    }

    endIndexDirty_ = false;
  }

  co_return endIndex_;
}

coro<bool> RocksNanoLog::co_setEndIndex(LogId endIndex) {
  endIndexDirty_ = true;

  auto key = fmt::format(KeyFormat::endIndexKey, versionId_);
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

coro<LogId> RocksNanoLog::trim(LogId logId) {
  auto startKey = fmt::format(KeyFormat::logKey, versionId_, 0);
  auto endKey = fmt::format(KeyFormat::logKey, versionId_, logId);
  auto deleteResult = co_await kvStore_->deleteRange(startKey, endKey);
  if (deleteResult) {
    co_return logId;
  }

  throw std::runtime_error{"log trim failed"};
}

} // namespace rk::projects::durable_log
