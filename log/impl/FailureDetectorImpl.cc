//
// Created by Rahul  Kushwaha on 3/11/23.
//

#include "FailureDetectorImpl.h"
#include "folly/executors/CPUThreadPoolExecutor.h"
#include "folly/executors/thread_factory/NamedThreadFactory.h"
#include <folly/experimental/coro/Collect.h>

namespace rk::projects::durable_log {

FailureDetectorImpl::FailureDetectorImpl
    (std::shared_ptr<HealthCheck> healthCheck,
     std::shared_ptr<VirtualLog> virtualLog,
     std::shared_ptr<folly::Executor> executor)
    : healthCheck_{std::move(healthCheck)},
      virtualLog_{std::move(virtualLog)},
      state_{std::make_unique<State>(State{-1})},
      ensembleAlive_{false},
      healthCheckRecords_{std::vector<bool>(5, false)},
      executor_{std::move(executor)} {
  runHealthCheckLoop().scheduleOn(executor_.get());
}

std::optional<MetadataConfig> FailureDetectorImpl::getLatestMetadataConfig() {
  return {};
}

bool FailureDetectorImpl::failure() {
  return false;
}

folly::coro::Task<void> FailureDetectorImpl::reconcile() {
  auto currentConfig = co_await virtualLog_->getCurrentConfig();
  while (!ensembleAlive_.load()) {
    // Reconfigure.
    auto newConfig = co_await virtualLog_->reconfigure(currentConfig);
    state_ = std::make_unique<State>(State{newConfig.version_id()});

    // Wait for some time to see if the ensemble is back up and alive.
    co_await folly::futures::sleep(std::chrono::seconds{1});
  }
}

folly::coro::Task<void> FailureDetectorImpl::runHealthCheckLoop() {
  // Wait for some milliseconds.
  co_await folly::futures::sleep(std::chrono::milliseconds{200});

  auto timeoutCoro =
      []() -> folly::coro::Task<bool> {
        co_await folly::futures::sleep(std::chrono::milliseconds{100});
        co_return false;
      };

  std::pair<std::size_t, folly::Try<bool>>
      coResult =
      co_await folly::coro::collectAny(healthCheck_->isAlive(),
                                       timeoutCoro());

  bool result = false;
  // HealthCheck Returned
  if (coResult.first == 0 && !coResult.second.hasException()) {
    result = coResult.second.value();
  }

  healthCheckRecords_.emplace_back(result);
  healthCheckRecords_.erase(healthCheckRecords_.begin());

  bool alive = true;
  for (auto healthCheckRecord: healthCheckRecords_) {
    alive &= healthCheckRecord;
  }

  ensembleAlive_.store(alive);
}

}