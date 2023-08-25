//
// Created by Rahul  Kushwaha on 8/24/23.
//
#pragma once

#include "folly/executors/CPUThreadPoolExecutor.h"
#include "folly/experimental/coro/AsyncGenerator.h"
#include "folly/experimental/coro/Task.h"
#include "folly/futures/Future.h"
#include <list>

namespace rk::projects::utils {

template <typename T> class FlatMapSequential {
public:
  explicit FlatMapSequential(std::shared_ptr<folly::Executor> executor)
      : mtx_{std::make_unique<std::mutex>()},
        condVar_{std::make_unique<std::condition_variable>()},
        executor_{std::move(executor)}, list_{}, drainAndStop_{false} {}

  folly::coro::AsyncGenerator<T &&> getGenerator() {
    while (true) {
      std::optional<folly::SemiFuture<T>> listElement;

      {
        std::unique_lock lg{*mtx_};
        condVar_->wait(lg,
                       [this]() { return !list_.empty() || drainAndStop_; });

        if (list_.empty() && drainAndStop_) {
          break;
        }

        if (list_.empty()) {
          continue;
        }

        auto future = std::move(list_.front());
        listElement.emplace(std::move(future));

        list_.pop_front();
      }

      assert(listElement.has_value());

      auto result = co_await std::move(listElement.value());
      co_yield std::move(result);
    }

    co_return;
  }

  void add(std::function<T()> func) {
    auto [promise, future] = folly::makePromiseContract<T>();

    auto lambda = [f = std::move(func), p = std::move(promise)]() mutable {
      auto result = f();
      p.setValue(std::move(result));
    };

    executor_->add(std::move(lambda));

    {
      std::lock_guard lg{*mtx_};

      list_.push_back(std::move(future));
      if (list_.size() == 1) {
        condVar_->notify_one();
      }
    }
  }

  void stop() {
    drainAndStop_ = true;
    std::unique_lock lg{*mtx_};
    condVar_->notify_one();
  }

  ~FlatMapSequential() { LOG(INFO) << "destruction"; }

private:
  std::unique_ptr<std::mutex> mtx_;
  std::unique_ptr<std::condition_variable> condVar_;
  std::shared_ptr<folly::Executor> executor_;
  std::list<folly::SemiFuture<T>> list_;
  bool drainAndStop_;
};

} // namespace rk::projects::utils