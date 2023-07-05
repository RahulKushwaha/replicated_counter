//
// Created by Rahul  Kushwaha on 12/31/22.
//

#include "InMemoryNanoLog.h"

namespace rk::projects::durable_log {

InMemoryNanoLog::InMemoryNanoLog(std::string id, std::string name,
                                       std::string metadataVersionId,
                                       LogId startIndex, LogId endIndex,
                                       bool sealed)
    : id_(std::move(id)), name_(std::move(name)),
      metadataVersionId_{std::move(metadataVersionId)}, startIndex_(startIndex),
      endIndex_(endIndex), sealed_(sealed), logs_{},
      completionQueue_{startIndex_} {}

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

  auto result = logs_.emplace(logId, std::move(logEntryPayload));

  if (!result.second) {
    if (skipSeal) {
      // TODO: Check if the logEntry is the same as provided in the call.
      co_return co_await folly::makeSemiFuture(logId);
    } else {
      co_return co_await folly::makeSemiFuture<LogId>(
          folly::make_exception_wrapper<NanoLogLogPositionAlreadyOccupied>());
    }
  }

  auto [promise, future] = folly::makePromiseContract<LogId>();
  completionQueue_.add(logId, std::move(promise), logId);
  if (globalCommitIndex.has_value()) {
    completionQueue_.completeAllBelow(globalCommitIndex.value());
  }

  co_return co_await std::move(future);
}

coro<std::variant<LogEntry, LogReadError>>
InMemoryNanoLog::getLogEntry(LogId logId) {
  if (auto itr = logs_.find(logId); itr != logs_.end()) {
    co_return {LogEntry{logId, itr->second}};
  }

  co_return {LogReadError::NotFound};
}

coro<LogId> InMemoryNanoLog::seal() {
  sealed_ = true;
  co_return completionQueue_.getCurrentIndex();
}

LogId InMemoryNanoLog::getStartIndex() { return startIndex_; }

LogId InMemoryNanoLog::getEndIndex() { return endIndex_; }

bool InMemoryNanoLog::isSealed() { return sealed_; }

coro<LogId> InMemoryNanoLog::getLocalCommitIndex() {
  co_return completionQueue_.getCurrentIndex();
}

} // namespace rk::projects::durable_log