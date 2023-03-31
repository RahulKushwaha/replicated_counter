//
// Created by Rahul  Kushwaha on 3/11/23.
//

#pragma once

#include "log/impl/EnsembleConfig.h"
#include "log/include/FailureDetector.h"
#include "log/include/HealthCheck.h"
#include "log/include/MetadataStore.h"
#include "log/include/VirtualLog.h"
#include "log/server/proto/ServerConfig.pb.h"

namespace rk::projects::durable_log {

class FailureDetectorImpl: public FailureDetector {
 public:
  explicit FailureDetectorImpl(std::shared_ptr<HealthCheck> healthCheck,
                               std::shared_ptr<VirtualLog> virtualLog,
                               std::shared_ptr<folly::Executor> executor,
                               rk::projects::server::ServerConfig logServerConfig);

 public:
  std::optional<MetadataConfig> getLatestMetadataConfig() override;
  bool healthy() override;
  folly::coro::Task<void> reconcileLoop() override;

  ~FailureDetectorImpl() override {
    LOG(INFO) << "Destructor Called";
    healthCheckLoopCancellationSource_.requestCancellation();
    reconciliationLoopCancellationSource_.requestCancellation();
  }

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
  rk::projects::server::ServerConfig logServerConfig_;

  std::vector<bool> healthCheckRecords_;
  std::shared_ptr<folly::Executor> executor_;

  folly::CancellationSource healthCheckLoopCancellationSource_;
  folly::CancellationSource reconciliationLoopCancellationSource_;
};

}
