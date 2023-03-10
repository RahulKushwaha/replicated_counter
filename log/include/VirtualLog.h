//
// Created by Rahul  Kushwaha on 12/29/22.
//
#pragma once

#include <string>
#include <folly/futures/Future.h>
#include "Common.h"

namespace rk::projects::durable_log {

class VirtualLog {
 public:
  virtual std::string getId() = 0;
  virtual std::string getName() = 0;
  virtual folly::SemiFuture<LogId> append(std::string logEntryPayload) = 0;
  virtual folly::SemiFuture<std::variant<LogEntry, LogReadError>>
  getLogEntry(LogId logId) = 0;
  virtual void reconfigure() = 0;
  virtual folly::SemiFuture<LogId> sync() = 0;

  virtual ~VirtualLog() = default;
};

}
