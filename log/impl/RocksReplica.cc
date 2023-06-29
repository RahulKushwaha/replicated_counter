//
// Created by Rahul  Kushwaha on 6/28/23.
//

#include "RocksReplica.h"
#include "fmt/format.h"
#include "folly/Conv.h"

namespace rk::projects::durable_log {
RocksReplica::RocksReplica(std::string id, std::string name,
                           std::shared_ptr<MetadataStore> metadataStore,
                           std::shared_ptr<persistence::KVStoreLite> kvStore)
    : id_{std::move(id)}, name_{std::move(name)},
      metadataStore_{std::move(metadataStore)}, kvStore_{std::move(kvStore)},
      completionQueueLookup_{} {}

std::string RocksReplica::getId() { return id_; }

std::string RocksReplica::getName() { return name_; }

coro<folly::Unit> RocksReplica::append(std::optional<LogId> globalCommitIndex,
                                       VersionId versionId, LogId logId,
                                       std::string logEntryPayload,
                                       bool skipSeal) {
  auto sealed = co_await kvStore_->get(getSealKey(versionId));
  if (!skipSeal && sealed.has_value()) {
    throw NanoLogSealedException(std::to_string(versionId));
  }
  if (sealed.has_value()) {
    LOG(INFO) << "sealed: " << sealed.value();
  } else {
    LOG(INFO) << "not sealed";
  }

  auto logKey = getLogKey(versionId, logId);
  auto append = co_await kvStore_->putIfNotExists(logKey, logEntryPayload);
  if (!append) {
    throw NanoLogLogPositionAlreadyOccupied();
  }

  utils::OrderedCompletionQueue<LogId> *completionQueue;
  if (auto itr = completionQueueLookup_.find(versionId);
      itr != completionQueueLookup_.end()) {
    completionQueue = itr->second.get();
  } else {
    auto config = metadataStore_->getConfig(versionId);
    if (!config.has_value()) {
      throw MetadataBlockNotFound{};
    }

    auto cq = std::make_unique<utils::OrderedCompletionQueue<LogId>>(
        config.value().start_index());
    auto result = completionQueueLookup_.emplace(versionId, std::move(cq));
    completionQueue = result.first->second.get();
  }

  auto [promise, future] = folly::makePromiseContract<LogId>();
  completionQueue->add(logId, std::move(promise), logId);
  if (globalCommitIndex.has_value()) {
    completionQueue->completeAllBelow(globalCommitIndex.value());
  }

  co_return co_await std::move(future).defer(
      [this, versionId, globalCommitIndex, completionQueue](auto &&) {
        auto commitIndex = globalCommitIndex.has_value()
                               ? globalCommitIndex.value()
                               : completionQueue->getCurrentIndex();
        return kvStore_
            ->put(getCommitIndexKey(versionId), std::to_string(commitIndex))
            .semi()
            .via(&folly::InlineExecutor::instance())
            .thenValue([](auto commitIndexWriteResult) -> folly::Unit {
              if (commitIndexWriteResult) {
                return folly::unit;
              }

              throw std::runtime_error("");
            });
      });
}

coro<std::variant<LogEntry, LogReadError>>
RocksReplica::getLogEntry(VersionId versionId, LogId logId) {
  auto value = co_await kvStore_->get(getLogKey(versionId, logId));
  if (!value.has_value()) {
    co_return LogReadError::NotFound;
  }

  co_return LogEntry{logId, std::move(value.value())};
}

LogId RocksReplica::getLocalCommitIndex(VersionId versionId) { return 5; }

LogId RocksReplica::seal(VersionId versionId) {
  auto sealKey = getSealKey(versionId);
  auto out = kvStore_->put(sealKey, "").semi().get();
  if (!out) {
    throw std::runtime_error{"seal operation could not be completed"};
  }

  if (auto optionalLogId = getLatestLogId(versionId).semi().get();
      optionalLogId.has_value()) {
    return optionalLogId.value();
  }

  return -1;
}

coro<std::optional<LogId>> RocksReplica::getLatestLogId(VersionId versionId) {
  auto generator = kvStore_->scan(getLogKeyPrefix(versionId));
  while (auto item = co_await generator.next()) {
    auto &&record = *item;
    auto key = std::get<0>(record);

    std::stringstream ss;
    for (std::size_t index = key.size() - 1; index > 0 && key[index] != '_';
         index--) {
      ss << key[index];
    }

    auto logId = folly::to<LogId>(ss.str());
    co_return logId;
  }

  co_return {};
}

std::string RocksReplica::getLogKey(VersionId versionId, LogId logId) {
  return fmt::format("LOG_ENTRY_{}_{}", versionId, logId);
}

std::string RocksReplica::getLogKeyPrefix(VersionId versionId) {
  return fmt::format("LOG_ENTRY_{}_", versionId);
}

std::string RocksReplica::getSealKey(VersionId versionId) {
  return fmt::format("SEAL_{}", versionId);
}

std::string RocksReplica::getCommitIndexKey(VersionId versionId) {
  return fmt::format("COMMIT_INDEX_{}", versionId);
}

} // namespace rk::projects::durable_log
