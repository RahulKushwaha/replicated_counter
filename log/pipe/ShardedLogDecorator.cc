//
// Created by Rahul  Kushwaha on 5/20/23.
//
#include "ShardedLogDecorator.h"

namespace rk::projects::durable_log {

std::string
ShardedLogDecorator::wrap(std::string shardId,
                          std::string payload) {
  ShardedLogEntry logEntry{};
  logEntry.set_shard_id(std::move(shardId));
  logEntry.set_payload(std::move(payload));

  return logEntry.SerializeAsString();
}

std::variant<ShardedLogEntry, LogReadError>
ShardedLogDecorator::unwrap(std::string payload) {
  ShardedLogEntry shardedLogEntry{};
  auto result = shardedLogEntry.ParseFromString(payload);

  if (!result) {
    return {LogReadError::ParsingError};
  }

  return shardedLogEntry;
}

}
