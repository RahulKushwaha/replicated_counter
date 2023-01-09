//
// Created by Rahul  Kushwaha on 12/31/22.
//
#pragma once

#include "Common.h"
#include <folly/futures/Future.h>

namespace rk::project::counter {

class Sequencer {
 public:
  virtual folly::SemiFuture<LogId> append(std::string logEntryPayload) = 0;
  virtual ~Sequencer() = default;
};

}