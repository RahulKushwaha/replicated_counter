//
// Created by Rahul  Kushwaha on 1/2/23.
//
#include <gtest/gtest.h>
#include <thread>
#include <future>
#include <folly/executors/ManualExecutor.h>

#include "../FutureUtils.h"

namespace rk::project::utils {

TEST(FutureUtilsTest, anyNSuccessful) {
  auto [promise, future] = folly::makePromiseContract<std::int32_t>();
  std::vector<folly::SemiFuture<std::int32_t >> futures;
  futures.emplace_back(std::move(future));

  folly::SemiFuture<std::int32_t>
      result = anyNSuccessful(std::move(futures), 1)
      .defer([](auto &&g) { return 5; });

  promise.setValue(54);
  ASSERT_EQ(std::move(result).get(), 5);
}

}