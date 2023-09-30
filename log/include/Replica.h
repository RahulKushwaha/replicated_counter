//
// Created by Rahul  Kushwaha on 12/31/22.
//

#pragma once

#include "Common.h"
#include "MetadataStore.h"

#include <folly/futures/Future.h>

#include <string>
#include <variant>

namespace rk::projects::durable_log {

class Replica {
 public:
  virtual std::string getId() = 0;
  virtual std::string getName() = 0;
  virtual coro<folly::Unit> append(std::optional<LogId> globalCommitIndex,
                                   VersionId versionId, LogId logId,
                                   std::string logEntryPayload,
                                   bool skipSeal = false) = 0;
  virtual coro<std::variant<LogEntry, LogReadError>> getLogEntry(
      VersionId versionId, LogId logId) = 0;

  virtual coro<LogId> getCommitIndex(VersionId versionId) = 0;
  virtual coro<LogId> seal(VersionId versionId) = 0;
  virtual coro<LogId> trim(VersionId versionId, LogId logId) = 0;

  virtual ~Replica() = default;
};

class MetadataBlockNotPresent : public std::exception {
 public:
  const char* what() const noexcept override {
    return "MetadataBlock containing the log_id is not present.";
  }
};

}  // namespace rk::projects::durable_log