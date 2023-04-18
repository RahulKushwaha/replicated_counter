//
// Created by Aman Mahajan on 4/17/23.
//

#include "log/include/VirtualLog.h"

namespace rk::projects::counter_app {

using namespace rk::projects::durable_log;

class InMemoryFakeVirtualLog: public durable_log::VirtualLog {
 public:
  explicit InMemoryFakeVirtualLog() : logEntries_{}, logId_{1} {
  }

  std::string getId() override { throw std::runtime_error{"NOT_IMPLEMENTED"}; }

  std::string getName() override {
    throw std::runtime_error{"NOT_IMPLEMENTED"};
  }

  folly::SemiFuture<durable_log::LogId>
  append(std::string logEntryPayload) override {
    auto currentLogId = logId_;
    logEntries_[currentLogId] = logEntryPayload;
    logId_++;
    return folly::makeSemiFuture(currentLogId);
  }

  folly::SemiFuture<std::variant<LogEntry, LogReadError>>
  getLogEntry(LogId logId) override {
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
    throw std::runtime_error{"NOT_IMPLEMENTED"};
  }

 private:
  std::map<durable_log::LogId, std::string> logEntries_;
  durable_log::LogId logId_;
};

}