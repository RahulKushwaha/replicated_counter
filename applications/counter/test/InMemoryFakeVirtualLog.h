//
// Created by Aman Mahajan on 4/17/23.
//

#include "log/include/VirtualLog.h"

namespace rk::projects::counter_app {

using namespace rk::projects::durable_log;

class InMemoryFakeVirtualLog : public durable_log::VirtualLog {
public:
  explicit InMemoryFakeVirtualLog()
      : logEntries_{}, logId_{1}, mtx_{std::make_unique<std::shared_mutex>()} {}

  std::string getId() override { throw std::runtime_error{"NOT_IMPLEMENTED"}; }

  std::string getName() override {
    throw std::runtime_error{"NOT_IMPLEMENTED"};
  }

  folly::SemiFuture<durable_log::LogId>
  append(std::string logEntryPayload) override {
    std::unique_lock lk{*mtx_};
    auto currentLogId = logId_;
    logEntries_[currentLogId] = logEntryPayload;
    logId_++;
    return folly::makeSemiFuture(currentLogId);
  }

  folly::SemiFuture<std::variant<LogEntry, LogReadError>>
  getLogEntry(LogId logId) override {
    std::shared_lock lk{*mtx_};

    std::variant<LogEntry, LogReadError> variant;
    if (auto itr = logEntries_.find(logId); itr != logEntries_.end()) {
      variant = LogEntry{logId, itr->second};
      return folly::makeSemiFuture(std::move(variant));
    } else {
      variant = LogReadError{LogReadError::Unknown};
      return folly::makeSemiFuture(std::move(variant));
    }
  };

  folly::coro::Task<MetadataConfig> getCurrentConfig() override {
    throw std::runtime_error{"NOT_IMPLEMENTED"};
  }

  folly::coro::Task<MetadataConfig>
  reconfigure(MetadataConfig metadataConfig) override {
    throw std::runtime_error{"NOT_IMPLEMENTED"};
  }

  folly::coro::Task<void> refreshConfiguration() override {
    throw std::runtime_error{"NOT_IMPLEMENTED"};
  }

  folly::SemiFuture<LogId> sync() override {
    std::shared_lock lg{*mtx_};

    return folly::makeSemiFuture(logId_);
  }

private:
  std::map<durable_log::LogId, std::string> logEntries_;
  std::unique_ptr<std::shared_mutex> mtx_;
  durable_log::LogId logId_;
};

} // namespace rk::projects::counter_app
