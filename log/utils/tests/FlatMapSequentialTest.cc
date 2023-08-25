//
// Created by Rahul  Kushwaha on 8/24/23.
//
#include "log/utils/FlatMapSequential.h"
#include "folly/experimental/coro/Task.h"
#include <gtest/gtest.h>

namespace rk::projects::utils {
namespace {

folly::coro::Task<std::vector<int>>
consumer(folly::coro::AsyncGenerator<int &&> generator) {
  std::vector<int> result;
  while (auto item = co_await generator.next()) {
    LOG(INFO) << "received: " << item.value();
    result.emplace_back(item.value());
  }

  co_return result;
}

} // namespace

TEST(FlatMapSequentialTest, addOneTask) {
  std::shared_ptr<folly::Executor> executor =
      std::make_shared<folly::CPUThreadPoolExecutor>(5);
  FlatMapSequential<int> flatMapSequential{executor};
  auto generator = flatMapSequential.getGenerator();
  flatMapSequential.add([]() {
    LOG(INFO) << "Task Executed";
    return 1;
  });

  flatMapSequential.stop();

  auto result = consumer(std::move(generator)).semi().get();
  ASSERT_EQ(result.size(), 1);
  ASSERT_TRUE(std::is_sorted(result.begin(), result.end()));
}

TEST(FlatMapSequentialTest, addMultipleTask) {
  int totalSize = 10;
  std::shared_ptr<folly::Executor> executor =
      std::make_shared<folly::CPUThreadPoolExecutor>(5);
  FlatMapSequential<int> flatMapSequential{executor};
  auto &&generator = flatMapSequential.getGenerator();
  // Add 10 tasks. Each with decreasing sleep time.
  for (int i = 0; i < totalSize; i++) {
    flatMapSequential.add([i]() {
      std::this_thread::sleep_for(std::chrono::milliseconds{(10 - i) * 10});
      LOG(INFO) << "Task Executed: " << i;
      return i;
    });
    LOG(INFO) << "Added: " << i;
  }

  flatMapSequential.stop();

  auto result = consumer(std::move(generator)).semi().get();
  ASSERT_EQ(result.size(), totalSize);
  ASSERT_TRUE(std::is_sorted(result.begin(), result.end()));
}

} // namespace rk::projects::utils