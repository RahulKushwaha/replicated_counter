//
// Created by Rahul  Kushwaha on 1/1/23.
//

#include <gtest/gtest.h>
#include <random>

#include "../VectorBasedNanoLog.h"

namespace rk::project::counter {

TEST(VectorBasedNanoLogTest, AppendWithoutSealDonotFail) {
  VectorBasedNanoLog log{"id", "name", "versionId", 4, 40, false};

  ASSERT_NO_THROW(log.append(4, "").get()) << "No Exception";
}

TEST(VectorBasedNanoLogTest, OutOfOrderAppendNeverFinishes) {
  VectorBasedNanoLog log{"id", "name", "versionId", 4, 40, false};
  auto future = log.append(5, "");

  ASSERT_FALSE(future.isReady());
}

TEST(VectorBasedNanoLogTest,
     OrderOfOrderAppendSucceedsAfterAllPreviousAreCompleted) {
  VectorBasedNanoLog log{"id", "name", "versionId", 4, 40, false};

  auto future6 = log.append(6, "");
  ASSERT_FALSE(future6.isReady());

  auto future5 = log.append(5, "");
  ASSERT_FALSE(future6.isReady());
  ASSERT_FALSE(future5.isReady());

  auto future4 = log.append(4, "");
  ASSERT_EQ(future4.value(), 4);
  ASSERT_EQ(future5.value(), 5);
  ASSERT_EQ(future6.value(), 6);
}

TEST(VectorBasedNanoLogTest, MultipleUnorderedAppends) {
  LogId startLogId = 4;
  LogId endLogId = 40;

  VectorBasedNanoLog
      log{"id", "name", "versionId", startLogId, endLogId, false};

  std::vector<LogId> logIds;
  for (LogId id = startLogId; id < endLogId; id++) {
    logIds.emplace_back(id);
  }

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  auto rng = std::default_random_engine{seed};
  std::shuffle(logIds.begin(), logIds.end(), rng);

  LogId pivot = logIds.back();
  logIds.pop_back();

  std::map<LogId, folly::SemiFuture<LogId>> futures;
  for (auto logId: logIds) {
    auto future = log.append(logId, "");

    futures.emplace(logId, std::move(future));
  }

  for (auto &[logId, future]: futures) {
    if (logId < pivot) {
      ASSERT_EQ(future.value(), logId);
    } else {
      ASSERT_THROW(future.value(), folly::FutureNotReady);
    }
  }

  ASSERT_EQ(log.getLocalCommitIndex(), pivot);
}

TEST(VectorBasedNanoLogTest, FailedAppendsAfterSeal) {
  VectorBasedNanoLog log{"id", "name", "versionId", 4, 40, false};
  log.seal();

  ASSERT_THROW(log.append(4, "").get(), NanoLogSealedException)
                << "Exception Thrown";
}

}
