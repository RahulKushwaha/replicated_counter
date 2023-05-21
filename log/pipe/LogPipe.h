//
// Created by Rahul  Kushwaha on 5/20/23.
//

#pragma once
#include "log/include/Common.h"
#include "folly/experimental/coro/Task.h"
#include "log/pipe/ShardedLogDecorator.h"
#include "log/pipe/ChecksumDecorator.h"
#include "log/pipe/LogReaderWriter.h"

namespace rk::projects::durable_log {

struct SLogEntry {
  std::string shardId;
  LogId logId;
  std::string payload;
};

class LogPipe {
 public:
  explicit LogPipe(std::shared_ptr<LogReaderWriter> log)
      : log_{std::move(log)} {}

  folly::coro::Task<void> append(std::string shardId, std::string payload) {
    auto slog =
        shardedLogDecorator_.wrap(std::move(shardId), std::move(payload));
    auto clog = checksumDecorator_.wrap(slog);

    co_await
    log_->append(clog);

    co_return;
  }

  folly::coro::Task<std::variant<SLogEntry, LogReadError>>
  getLogEntry(LogId logId) {
    auto logEntryResponse = co_await
    log_->getLogEntry(logId);

    if (std::holds_alternative<LogReadError>(logEntryResponse)) {
      co_return std::get<LogReadError>(logEntryResponse);
    }

    auto logEntry = std::get<LogEntry>(logEntryResponse);

    auto cEntryResponse = checksumDecorator_.unwrap(logEntry.payload);
    if (std::holds_alternative<LogReadError>(cEntryResponse)) {
      co_return std::get<LogReadError>(cEntryResponse);
    }

    auto cEntry = std::get<ChecksumProtectedLogEntry>(cEntryResponse);

    auto sEntryResponse = shardedLogDecorator_.unwrap(cEntry.payload());
    if (std::holds_alternative<LogReadError>(sEntryResponse)) {
      co_return std::get<LogReadError>(sEntryResponse);
    }

    auto sEntry = std::get<ShardedLogEntry>(sEntryResponse);
    co_return{SLogEntry{
        .shardId = sEntry.shard_id(),
        .logId = logId,
        .payload = sEntry.payload(),
    }};
  }

 private:
  std::shared_ptr<LogReaderWriter> log_;
  ShardedLogDecorator shardedLogDecorator_;
  ChecksumDecorator checksumDecorator_;
};

}