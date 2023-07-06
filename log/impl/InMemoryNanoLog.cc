//
// Created by Rahul  Kushwaha on 12/31/22.
//

#include "InMemoryNanoLog.h"

namespace rk::projects::durable_log {

InMemoryNanoLog::InMemoryNanoLog(std::string id, std::string name,
                                 std::string metadataVersionId,
                                 LogId startIndex, LogId endIndex, bool sealed)
    : id_(std::move(id)), name_(std::move(name)),
      metadataVersionId_{std::move(metadataVersionId)}, startIndex_(startIndex),
      endIndex_(endIndex), sealed_(sealed), logs_{},
      completionQueue_{startIndex_}, mtx_{std::make_shared<std::mutex>()} {}

std::string InMemoryNanoLog::getId() { return id_; }

std::string InMemoryNanoLog::getName() { return name_; }

std::string InMemoryNanoLog::getMetadataVersionId() {
  return metadataVersionId_;
}

coro<LogId> InMemoryNanoLog::append(std::optional<LogId> globalCommitIndex,
                                    LogId logId, std::string logEntryPayload,
                                    bool skipSeal) {
  if (!skipSeal && sealed_) {
    co_return co_await folly::makeSemiFuture<LogId>(
        folly::make_exception_wrapper<NanoLogSealedException>(
            metadataVersionId_));
  }

  auto [promise, future] = folly::makePromiseContract<LogId>();
  {
    std::lock_guard lg{*mtx_};

    auto result = logs_.emplace(logId, std::move(logEntryPayload));
    if (!result.second) {
      if (skipSeal) {
        // TODO: Check if the logEntry is the same as provided in the call.
        co_return logId;
      } else {
        co_return co_await folly::makeSemiFuture<LogId>(
            folly::make_exception_wrapper<NanoLogLogPositionAlreadyOccupied>());
      }
    }

    completionQueue_.add(logId, std::move(promise), logId);
    if (globalCommitIndex.has_value()) {
      completionQueue_.completeAllBelow(globalCommitIndex.value());
    }
  }

  co_return co_await std::move(future);
}

coro<std::variant<LogEntry, LogReadError>>
InMemoryNanoLog::getLogEntry(LogId logId) {
  std::lock_guard lg{*mtx_};
  if (auto itr = logs_.find(logId); itr != logs_.end()) {
    co_return {LogEntry{logId, itr->second}};
  }

  co_return {LogReadError::NotFound};
}

coro<LogId> InMemoryNanoLog::seal() {
  std::lock_guard lg{*mtx_};
  sealed_ = true;
  co_return completionQueue_.getCurrentIndex();
}

LogId InMemoryNanoLog::getStartIndex() { return startIndex_; }

LogId InMemoryNanoLog::getEndIndex() { return endIndex_; }

bool InMemoryNanoLog::isSealed() { return sealed_; }

coro<LogId> InMemoryNanoLog::getLocalCommitIndex() {
  std::lock_guard lg{*mtx_};
  co_return completionQueue_.getCurrentIndex();
}

} // namespace rk::projects::durable_log