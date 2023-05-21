//
// Created by Rahul  Kushwaha on 5/20/23.
//
#pragma once
#include "folly/experimental/coro/Task.h"
#include "log/include/Common.h"
#include "log/include/VirtualLog.h"

namespace rk::projects::durable_log {

class LogReaderWriter {
 public:
  explicit LogReaderWriter(std::shared_ptr<VirtualLog> virtualLog)
      : virtualLog_{std::move(virtualLog)} {}

  folly::coro::Task<void> append(std::string payload) {
    co_await
    virtualLog_->append(std::move(payload));

    co_return;
  }

  folly::coro::Task<std::variant<LogEntry, LogReadError>>
  getLogEntry(LogId logId) {
    auto result = co_await
    virtualLog_->getLogEntry(logId);

    co_return result;
  }

 private:
  std::shared_ptr<VirtualLog> virtualLog_;
};

}