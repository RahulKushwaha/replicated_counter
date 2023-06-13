//
// Created by Rahul  Kushwaha on 1/2/23.
//

#include <chrono>
#include <folly/futures/Future.h>
#include <gtest/gtest.h>
#include <map>
#include <random>

#include "../OrderedCompletionQueue.h"

namespace rk::projects::utils {

TEST(OrderedCompletionQueueTests, AddFirstElement) {
  OrderedCompletionQueue<std::int32_t> queue;

  folly::Promise<std::int32_t> promise;
  folly::Future<std::int32_t> future = promise.getFuture();
  queue.add(0, std::move(promise), 4);

  ASSERT_EQ(future.value(), 4);
}

TEST(OrderedCompletionQueueTests,
     GetCurrentIndexReturnsFirstIncompletePromise) {
  {
    OrderedCompletionQueue<std::int32_t> queue;
    queue.add(1, folly::Promise<std::int32_t>{}, 1);
    ASSERT_EQ(queue.getCurrentIndex(), 0);
  }

  {
    OrderedCompletionQueue<std::int32_t> queue;
    queue.add(0, folly::Promise<std::int32_t>{}, 0);
    queue.add(2, folly::Promise<std::int32_t>{}, 2);
    queue.add(1, folly::Promise<std::int32_t>{}, 1);
    ASSERT_EQ(queue.getCurrentIndex(), 3);
  }

  {
    OrderedCompletionQueue<std::int32_t> queue;
    queue.add(0, folly::Promise<std::int32_t>{}, 0);
    queue.add(2, folly::Promise<std::int32_t>{}, 2);
    queue.add(1, folly::Promise<std::int32_t>{}, 1);
    queue.add(4, folly::Promise<std::int32_t>{}, 4);
    ASSERT_EQ(queue.getCurrentIndex(), 3);
  }
}

TEST(OrderedCompletionQueueTests, AddElementsInSequence) {
  OrderedCompletionQueue<std::int32_t> queue;

  for (int i = 0; i < 100; i++) {
    folly::Promise<std::int32_t> promise;
    folly::Future<std::int32_t> future = promise.getFuture();
    queue.add(i, std::move(promise), i);

    ASSERT_EQ(future.value(), i);
  }
}

TEST(OrderedCompletionQueueTests, AddElementsRandomlyAndCompleteOld) {
  GTEST_SKIP() << "Skipping single test";
  OrderedCompletionQueue<std::int32_t> queue;
  std::vector<std::int32_t> elements{1, 3, 5, 7, 8, 56};

  std::vector<folly::Future<std::int32_t>> futures;
  for (auto element : elements) {
    folly::Promise<std::int32_t> promise;
    folly::Future<std::int32_t> future = promise.getFuture();

    queue.add(element, std::move(promise), element);

    futures.emplace_back(std::move(future));
  }

  // Add 57 which completes all previous ones.
  {
    folly::Promise<std::int32_t> promise;
    folly::Future<std::int32_t> future = promise.getFuture();

    //    queue.addAndCompleteOld(57, std::move(promise), 57);
  }

  folly::collectAll(futures.begin(), futures.end()).get();

  for (auto &future : futures) {
    ASSERT_TRUE(future.hasValue());
  }
}

TEST(OrderedCompletionQueueTests, AddElementsRandomly) {
  OrderedCompletionQueue<std::int32_t> queue;
  std::vector<std::int32_t> elements;
  for (std::int32_t i = 0; i < 100; i++) {
    elements.push_back(i);
  }

  auto rng = std::default_random_engine{};
  std::shuffle(std::begin(elements), std::end(elements), rng);

  std::vector<folly::Future<std::int32_t>> futures;
  for (auto element : elements) {
    folly::Promise<std::int32_t> promise;
    folly::Future<std::int32_t> future = promise.getFuture();

    queue.add(element, std::move(promise), element);

    futures.emplace_back(std::move(future));
  }

  folly::collectAll(futures.begin(), futures.end()).get();

  for (auto &future : futures) {
    ASSERT_TRUE(future.hasValue());
  }
}

TEST(OrderedCompletionQueueTests,
     AddElementsRandomlyAndCheckCompletedVsIncompleted) {
  for (std::int32_t iteration = 0; iteration < 1000; iteration++) {
    OrderedCompletionQueue<std::int32_t> queue;
    std::vector<std::int32_t> elements;
    for (std::int32_t i = 0; i < 100; i++) {
      elements.push_back(i);
    }

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    auto rng = std::default_random_engine{seed};
    std::shuffle(std::begin(elements), std::end(elements), rng);

    std::int32_t pivot = elements.back();
    elements.pop_back();

    // Now all the futures before the pivot should be completed.
    // And all the future after the pivot should be pending.

    std::map<std::int32_t, folly::Future<std::int32_t>> futures;
    for (auto element : elements) {
      folly::Promise<std::int32_t> promise;
      folly::Future<std::int32_t> future = promise.getFuture();

      queue.add(element, std::move(promise), element);

      futures.emplace(element, std::move(future));
    }

    std::int32_t complete = 0, incomplete = 0;
    for (auto &[index, future] : futures) {
      if (index < pivot) {
        ASSERT_EQ(future.value(), index);
        complete++;
      } else {
        ASSERT_FALSE(future.isReady());
        incomplete++;
      }
    }

    ASSERT_EQ(complete, pivot);
    ASSERT_EQ(incomplete, futures.size() - pivot);
    ASSERT_EQ(queue.getCurrentIndex(), pivot);
    LOG(INFO) << "Total: " << futures.size() << " "
              << "Complete: " << complete << " "
              << "Incomplete: " << incomplete;
  }
}

} // namespace rk::projects::utils