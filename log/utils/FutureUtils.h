//
// Created by Rahul  Kushwaha on 1/2/23.
//

#pragma once

#include <folly/executors/InlineExecutor.h>
#include <folly/futures/Future.h>
#include <vector>

namespace rk::projects::utils {
class MultipleExceptions : public std::exception {
public:
  MultipleExceptions() : state_{std::make_shared<State>()} {}

  void add(std::size_t index, const std::exception_ptr &exceptionPtr) {
    std::lock_guard lockGuard{state_->mtx};
    state_->exceptions_.emplace_back(index, exceptionPtr);
  }

  std::vector<std::pair<std::size_t, std::exception_ptr>> exceptions() {
    std::lock_guard lockGuard{state_->mtx};
    return state_->exceptions_;
  }

  std::string getDebugString() {
    std::lock_guard lockGuard{state_->mtx};
    std::stringstream ss;
    for (auto &[index, ex] : state_->exceptions_) {
      try {
        std::rethrow_exception(ex);
      } catch (const std::exception &e) {
        ss << "Future Index: " << index << " " << e.what() << std::endl;
      }
    }

    return ss.str();
  }

private:
  struct State {
    std::mutex mtx;
    std::vector<std::pair<std::size_t, std::exception_ptr>> exceptions_;
  };

  std::shared_ptr<State> state_;
};

template <typename T>
folly::SemiFuture<folly::Unit>
anyNSuccessful(std::vector<folly::SemiFuture<T>> futures, std::int32_t n) {
  std::shared_ptr<folly::Promise<folly::Unit>> promise =
      std::make_shared<folly::Promise<folly::Unit>>();

  std::shared_ptr<std::atomic_int32_t> successCounter =
      std::make_shared<std::atomic_int32_t>(0);
  std::shared_ptr<std::atomic_int32_t> failureCounter =
      std::make_shared<std::atomic_int32_t>(0);

  std::shared_ptr<MultipleExceptions> multipleExceptions =
      std::make_shared<MultipleExceptions>();

  for (std::size_t index = 0; index < futures.size(); index++) {
    auto &future = futures[index];
    std::move(future)
        .via(&folly::InlineExecutor::instance())
        .then([successCounter, failureCounter, index, n, promise,
               totalPromises = futures.size(),
               multipleExceptions](folly::Try<T> &&result) {
          auto successValue = successCounter.get();
          auto failedValue = failureCounter.get();
          if (result.hasValue()) {
            (*successValue)++;
          } else {
            multipleExceptions->add(index,
                                    result.exception().to_exception_ptr());
            (*failedValue)++;
          }

          if (*successCounter == n) {
            promise->template setValue();
          } else if (*successValue + *failedValue == totalPromises) {
            promise->setException(*multipleExceptions);
          }

          return std::forward<decltype(result)>(result);
        });
  }

  return promise->getSemiFuture();
}

} // namespace rk::projects::utils
