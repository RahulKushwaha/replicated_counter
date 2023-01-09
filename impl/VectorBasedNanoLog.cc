//
// Created by Rahul  Kushwaha on 12/31/22.
//

#include "VectorBasedNanoLog.h"

namespace rk::project::counter {

VectorBasedNanoLog::VectorBasedNanoLog(
    std::string id,
    std::string name,
    std::string metadataVersionId,
    LogId startIndex,
    LogId endIndex,
    bool sealed)
    : id_(std::move(id)), name_(std::move(name)),
      metadataVersionId_{std::move(metadataVersionId)},
      startIndex_(startIndex),
      endIndex_(endIndex),
      sealed_(sealed), logs_{},
      completionQueue_{startIndex_} {}

std::string VectorBasedNanoLog::getId() {
  return id_;
}

std::string VectorBasedNanoLog::getName() {
  return name_;
}

std::string VectorBasedNanoLog::getMetadataVersionId() {
  return metadataVersionId_;
}

folly::SemiFuture<LogId> VectorBasedNanoLog::append(LogId logId,
                                                    std::string logEntryPayload,
                                                    bool skipSeal) {
  if (!skipSeal && sealed_) {
    return folly::makeSemiFuture<LogId>(
        folly::make_exception_wrapper<NanoLogSealedException>());
  }

  auto result = logs_.emplace(logId, std::move(logEntryPayload));

  if (!result.second) {
    return folly::makeSemiFuture<LogId>(folly::make_exception_wrapper<
        NanoLogLogPositionAlreadyOccupied>());
  }

  auto [promise, future] = folly::makePromiseContract<LogId>();
  completionQueue_.add(logId, std::move(promise), logId);

  return std::move(future);
}

std::variant<LogEntry, LogReadError>
VectorBasedNanoLog::getLogEntry(LogId logId) {
  if (auto itr = logs_.find(logId); itr != logs_.end()) {
    return {LogEntry{logId, itr->second}};
  }

  return {LogReadError::NotFound};
}

LogId VectorBasedNanoLog::seal() {
  sealed_ = true;
  return completionQueue_.getCurrentIndex();
}

LogId VectorBasedNanoLog::getStartIndex() {
  return startIndex_;
}

LogId VectorBasedNanoLog::getEndIndex() {
  return endIndex_;
}

bool VectorBasedNanoLog::isSealed() {
  return sealed_;
}

LogId VectorBasedNanoLog::getLocalCommitIndex() {
  return completionQueue_.getCurrentIndex();
}

}