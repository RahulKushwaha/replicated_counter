//
// Created by Rahul  Kushwaha on 7/17/23.
//
#pragma once
#include "log/include/VirtualLog.h"
#include "log/proto/LogEntry.pb.h"
#include "log/utils/FlatMapSequential.h"
#include "statemachine/NullStateMachine.h"
#include "statemachine/include/StateMachine.h"

namespace rk::projects::state_machine {
using namespace std::chrono_literals;
using namespace rk::projects::durable_log;

template <typename R>
class VirtualLogStateMachine : public StateMachine<LogEntry_1, R> {
private:
  struct State {
    LogId logId;
    LogEntry_1 log;
  };

public:
  explicit VirtualLogStateMachine(std::shared_ptr<VirtualLog> virtualLog,
                                  LogId lastAppliedLogId = 0)
      : virtualLog_{std::move(virtualLog)},
        upstreamStateMachine_{
            std::make_shared<NullStateMachine<LogEntry_1, R>>()},
        lastFetchId_{0}, targetLogId_{-1}, lastAppliedLogId_{lastAppliedLogId},
        flatMapSeq_{std::make_unique<utils::FlatMapSequential<State>>(
            std::make_shared<folly::CPUThreadPoolExecutor>(10))},
        singleExecutor_{std::make_unique<folly::CPUThreadPoolExecutor>(1)},
        mtx_{std::make_unique<std::mutex>()},
        condVar_{std::make_unique<std::condition_variable>()}, promises_{},
        producerExecutor_{std::make_unique<folly::CPUThreadPoolExecutor>(1)},
        consumerExecutor_{std::make_unique<folly::CPUThreadPoolExecutor>(1)},
        producerLoopCancellationSource_{}, consumerLoopCancellationSource_{} {
    folly::coro::co_withCancellation(consumerLoopCancellationSource_.getToken(),
                                     consumeLoop())
        .scheduleOn(consumerExecutor_.get())
        .start();

    folly::coro::co_withCancellation(producerLoopCancellationSource_.getToken(),
                                     producerLoop())
        .scheduleOn(producerExecutor_.get())
        .start();
  }

  coro<R> append(LogEntry_1 t) override {
    auto logId = co_await virtualLog_->append(t.SerializeAsString());

    auto [promise, future] = folly::makePromiseContract<R>();
    {
      std::lock_guard lg{*mtx_};
      promises_.emplace(logId, std::move(promise));
      targetLogId_ = std::max(logId, targetLogId_);
      condVar_->notify_one();
    }

    co_return co_await std::move(future);
  }

  coro<R> apply(rk::projects::durable_log::LogEntry_1 t) override {
    co_return co_await upstreamStateMachine_->apply(std::move(t));
  }

  coro<void> sync() override {
    auto logId = co_await virtualLog_->sync() - 1;

    auto [promise, future] = folly::makePromiseContract<R>();
    {
      std::lock_guard lk{*mtx_};

      if (logId <= lastAppliedLogId_) {
        co_return;
      }

      promises_.emplace(logId, std::move(promise));
      targetLogId_ = std::max(logId, targetLogId_);
      condVar_->notify_one();
    }

    co_await std::move(future);
  }

  void setApplicator(
      std::shared_ptr<Applicator<LogEntry_1, R>> applicator) override {}

  void setUpstreamStateMachine(std::shared_ptr<StateMachine<LogEntry_1, R>>
                                   upstreamStateMachine) override {
    upstreamStateMachine_ = std::move(upstreamStateMachine);
  }

  ~VirtualLogStateMachine() {
    producerLoopCancellationSource_.requestCancellation();
    consumerLoopCancellationSource_.requestCancellation();

    producerExecutor_->stop();
    consumerExecutor_->stop();
    singleExecutor_->stop();
  }

private:
  coro<void> producerLoop() {
    LogId to = 0;
    while (true) {
      {
        std::unique_lock lk{*mtx_};
        condVar_->wait(lk, [to, this]() { return to < targetLogId_; });

        to = targetLogId_;
      }
      co_await folly::coro::co_reschedule_on_current_executor;

      while (lastFetchId_ + 1 <= to) {
        auto nextLogId = lastFetchId_ + 1;
        flatMapSeq_->add([this, nextLogId]() -> coro<State> {
          auto nextLog = co_await virtualLog_->getLogEntry(nextLogId);

          if (std::holds_alternative<durable_log::LogReadError>(nextLog)) {
            auto logReadError = std::get<durable_log::LogReadError>(nextLog);
            throw std::runtime_error{"log read error"};
          }

          durable_log::LogEntry serializedLogEntry =
              std::get<LogEntry>(nextLog);
          LogEntry_1 logEntry{};
          if (auto parseResult =
                  logEntry.ParseFromString(serializedLogEntry.payload);
              !parseResult) {
            throw std::runtime_error{"protobuf parse result"};
          }

          co_return State{nextLogId, logEntry};
        });

        lastFetchId_ = nextLogId;
      }
    }
  }

  coro<void> consumeLoop() {
    auto generator = flatMapSeq_->getGenerator();

    while (auto v = co_await generator.next()) {
      auto task = folly::coro::co_invoke([this, state = *v]() -> coro<void> {
        auto result = co_await apply(state.log);
        {
          std::lock_guard lg{*mtx_};
          auto itr = promises_.find(state.logId);
          while (itr != promises_.end() && itr->first == state.logId) {
            itr->second.setValue(result);
            itr++;
          }

          promises_.erase(state.logId);
          assert(lastAppliedLogId_ + 1 == state.logId &&
                 "applied logId should always be greater than the last one.");
          lastAppliedLogId_ = state.logId;
        }

        co_return;
      });

      std::move(task).scheduleOn(singleExecutor_.get()).start();
    }
  }

private:
  std::shared_ptr<VirtualLog> virtualLog_;
  std::shared_ptr<StateMachine<LogEntry_1, R>> upstreamStateMachine_;
  LogId lastFetchId_;
  LogId targetLogId_;
  LogId lastAppliedLogId_;
  std::shared_ptr<utils::FlatMapSequential<State>> flatMapSeq_;
  std::unique_ptr<folly::CPUThreadPoolExecutor> singleExecutor_;
  std::unique_ptr<std::mutex> mtx_;
  std::unique_ptr<std::condition_variable> condVar_;

  std::unique_ptr<folly::CPUThreadPoolExecutor> producerExecutor_;
  std::unique_ptr<folly::CPUThreadPoolExecutor> consumerExecutor_;
  std::multimap<LogId, folly::Promise<R>> promises_;

  folly::CancellationSource producerLoopCancellationSource_;
  folly::CancellationSource consumerLoopCancellationSource_;
};

} // namespace rk::projects::state_machine