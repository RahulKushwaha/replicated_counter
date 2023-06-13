//
// Created by Rahul  Kushwaha on 3/11/23.
//

#include "FailureDetectorImpl.h"
#include "folly/executors/CPUThreadPoolExecutor.h"
#include "folly/executors/thread_factory/NamedThreadFactory.h"
#include "folly/experimental/coro/Collect.h"
#include "folly/experimental/coro/Timeout.h"
#include "metrics/MetricsRegistry.h"
#include "prometheus/counter.h"

namespace rk::projects::durable_log {

auto &reconfiguration =
    prometheus::BuildCounter()
        .Name("reconfiguration")
        .Help("ensemble reconfiguration")
        .Register(metrics::MetricsRegistry::instance().registry());

auto &reconfigurationSuccess = reconfiguration.Add({{"result", "success"}});
auto &reconfigurationFailure = reconfiguration.Add({{"result", "failure"}});

FailureDetectorImpl::FailureDetectorImpl(
    std::shared_ptr<HealthCheck> healthCheck,
    std::shared_ptr<VirtualLog> virtualLog,
    std::shared_ptr<folly::Executor> executor,
    rk::projects::server::ServerConfig logServerConfig)
    : healthCheck_{std::move(healthCheck)}, virtualLog_{std::move(virtualLog)},
      state_{std::make_unique<State>(State{-1})}, ensembleAlive_{false},
      logServerConfig_{std::move(logServerConfig)},
      healthCheckRecords_{std::vector<bool>(5, false)},
      executor_{std::move(executor)}, healthCheckLoopCancellationSource_{},
      reconciliationLoopCancellationSource_{} {
  folly::coro::co_withCancellation(
      healthCheckLoopCancellationSource_.getToken(), runHealthCheckLoop())
      .scheduleOn(executor_.get())
      .start();

  folly::coro::co_withCancellation(
      reconciliationLoopCancellationSource_.getToken(), reconcileLoop())
      .scheduleOn(executor_.get())
      .start();
}

std::optional<MetadataConfig> FailureDetectorImpl::getLatestMetadataConfig() {
  return {};
}

bool FailureDetectorImpl::healthy() { return ensembleAlive_.load(); }

folly::coro::Task<void> FailureDetectorImpl::reconcileLoop() {
  while (true) {
    LOG(INFO) << "Reconciliation Loop";
    // Wait for some time to see if the ensemble is back up and alive.
    co_await folly::futures::sleep(std::chrono::milliseconds{100});

    if (!ensembleAlive_.load()) {
      LOG(INFO) << "Ensemble is not alive. Trigger Reconfiguration";
      // Reconfigure.
      MetadataConfig config{};
      config.mutable_sequencer_config()->set_id(
          logServerConfig_.sequencer_config().id());
      for (const auto &replica : logServerConfig_.replica_set()) {
        config.mutable_replica_set_config()->Add()->set_id(replica.id());
      }

      try {
        co_await virtualLog_->reconfigure(config);
        reconfigurationSuccess.Increment();
      } catch (const std::exception &e) {
        LOG(ERROR) << "failed to reconfigure: " << e.what();
        reconfigurationFailure.Increment();
      }
    }

    // Wait for some time to see if the ensemble is back up and alive.
    co_await folly::futures::sleep(std::chrono::milliseconds{5000});
  }
}

folly::coro::Task<void> FailureDetectorImpl::runHealthCheckLoop() {
  for (std::int64_t i = 0; i >= 0; i++) {
    // Wait for some milliseconds.
    co_await folly::futures::sleep(std::chrono::milliseconds{200});
    LOG(INFO) << "====Iteration: " << i << " Woke up after sleeping for " << 200
              << "ms====";
    LOG(INFO) << "System Alive: " << (ensembleAlive_.load() ? "T" : "F");

    auto startTime = std::chrono::steady_clock::now();

    auto timeoutCoro = folly::coro::co_invoke([]() -> folly::coro::Task<bool> {
      co_await folly::futures::sleep(std::chrono::milliseconds{200});
      co_return false;
    });

    std::pair<std::size_t, folly::Try<bool>> coResult =
        co_await folly::coro::collectAny(healthCheck_->isAlive(),
                                         std::move(timeoutCoro));

    auto diff = std::chrono::steady_clock::now() - startTime;
    auto diffInMs = std::chrono::duration_cast<std::chrono::milliseconds>(diff);

    if (auto sleepTime = std::chrono::milliseconds{200} - diffInMs;
        sleepTime > std::chrono::milliseconds{0}) {
      LOG(INFO) << "HealthCheck Ended. Sleeping for: " << sleepTime.count()
                << "ms";
      co_await folly::futures::sleep(sleepTime);
    }

    bool result = false;
    // HealthCheck Returned
    if (coResult.first == 0 && !coResult.second.hasException()) {
      result = coResult.second.value();
    }

    healthCheckRecords_.emplace_back(result);
    healthCheckRecords_.erase(healthCheckRecords_.begin());

    bool alive = false;
    for (auto healthCheckRecord : healthCheckRecords_) {
      alive |= healthCheckRecord;
    }

    ensembleAlive_.store(alive);

    if (!ensembleAlive_.load()) {
      co_await folly::coro::collectAny(
          folly::coro::co_invoke([]() -> folly::coro::Task<void> {
            co_await folly::futures::sleep(std::chrono::milliseconds{1000});
          }),
          virtualLog_->refreshConfiguration());
    }
  }
}
} // namespace rk::projects::durable_log