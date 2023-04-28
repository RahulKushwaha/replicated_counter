//
// Created by Rahul  Kushwaha on 12/31/22.
//
#pragma once

#include "Common.h"
#include <folly/futures/Future.h>

namespace rk::projects::durable_log {

class Sequencer {
 public:
  virtual std::string getId() = 0;
  virtual folly::SemiFuture<LogId> append(std::string logEntryPayload) = 0;
  virtual folly::SemiFuture<LogId> latestAppendPosition() = 0;
  virtual void start(VersionId versionId, LogId sequenceNum) = 0;
  virtual bool isAlive() = 0;
  virtual void stop() = 0;
  virtual ~Sequencer() = default;
};

}