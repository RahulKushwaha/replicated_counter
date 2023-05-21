//
// Created by Rahul  Kushwaha on 5/20/23.
//

#pragma once
#include "log/include/Common.h"
#include "folly/experimental/coro/Task.h"
#include "log/pipe/proto/ShardedLogEntry.pb.h"
#include "log/pipe/LogReaderWriter.h"

namespace rk::projects::durable_log {

class ShardedLogDecorator {
 public:
  std::string wrap(std::string shardId, std::string payload);
  std::variant<ShardedLogEntry, LogReadError> unwrap(std::string payload);
};

}