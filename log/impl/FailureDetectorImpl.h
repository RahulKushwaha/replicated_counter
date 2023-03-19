//
// Created by Rahul  Kushwaha on 3/11/23.
//

#pragma once

#include "log/include/FailureDetector.h"
#include "log/include/VirtualLog.h"
#include "log/include/MetadataStore.h"
#include "log/include/HealthCheck.h"

namespace rk::projects::durable_log {

class FailureDetectorImpl: public FailureDetector {
 public:
  explicit FailureDetectorImpl(std::shared_ptr<HealthCheck> healthCheck,
                               std::shared_ptr<VirtualLog> virtualLog,
                               std::shared_ptr<folly::Executor> executor);

 public:
  std::optional<MetadataConfig> getLatestMetadataConfig() override;
  bool failure() override;
  folly::coro::Task<void> reconcile() override;

 private:

  folly::coro::Task<void> runHealthCheckLoop();

 private:
  struct State {
    VersionId versionId;
  };

  std::shared_ptr<HealthCheck> healthCheck_;
  std::shared_ptr<VirtualLog> virtualLog_;
  std::unique_ptr<State> state_;
  std::atomic_bool ensembleAlive_;

  std::vector<bool> healthCheckRecords_;
  std::shared_ptr<folly::Executor> executor_;
};

}
