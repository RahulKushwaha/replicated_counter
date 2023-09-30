//
// Created by Rahul  Kushwaha on 12/29/22.
//

#pragma once

#include "Common.h"
#include "NanoLog.h"
#include "log/proto/MetadataConfig.pb.h"

#include <optional>

namespace rk::projects::durable_log {

struct OptimisticConcurrencyException : public std::exception {
  const char* what() const noexcept override {
    return "Optimistic Concurrency Exception";
  }
};

class MetadataStore {
 public:
  virtual coro<std::optional<MetadataConfig>> getConfig(
      VersionId versionId) = 0;

  virtual coro<std::optional<MetadataConfig>> getConfigUsingLogId(
      LogId logId) = 0;

  virtual coro<VersionId> getCurrentVersionId() = 0;

  virtual coro<void> compareAndAppendRange(
      MetadataConfig newMetadataConfig) = 0;

  virtual void printConfigChain() = 0;

  virtual ~MetadataStore() = default;
};

}  // namespace rk::projects::durable_log