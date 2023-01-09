//
// Created by Rahul  Kushwaha on 1/2/23.
//

#ifndef LOGSTORAGE_FUTUREUTILS_H
#define LOGSTORAGE_FUTUREUTILS_H

#include <folly/futures/Future.h>
#include <folly/executors/InlineExecutor.h>

namespace rk::project::utils {

template<typename T>
folly::SemiFuture<folly::Unit>
anyNSuccessful(std::vector<folly::SemiFuture<T>> futures,
               std::int32_t n) {
  std::shared_ptr<folly::Promise<folly::Unit>>
      promise = std::make_shared<folly::Promise<folly::Unit>>();

  std::shared_ptr<std::atomic_int32_t>
      successCounter = std::make_shared<std::atomic_int32_t>(0);
  std::shared_ptr<std::atomic_int32_t>
      failureCounter = std::make_shared<std::atomic_int32_t>(0);

  for (auto &future: futures) {
    std::move(future)
        .via(&folly::InlineExecutor::instance())
        .then([successCounter, failureCounter,
                  n, promise,
                  totalPromises =
                  futures.size()](folly::Try<T> &&result) {
          auto successValue = successCounter.get();
          auto failedValue = failureCounter.get();
          if (result.hasValue()) {
            (*successValue)++;
          } else {
            (*failedValue)++;
          }

          if (*successCounter == n) {
            promise->template setValue();
          } else if (*successValue + *failedValue == totalPromises) {
            promise->setException(std::exception{});
          }

          return std::forward<decltype(result)>(result);
        });
  }


  return promise->getSemiFuture();
}

}

#endif //LOGSTORAGE_FUTUREUTILS_H
