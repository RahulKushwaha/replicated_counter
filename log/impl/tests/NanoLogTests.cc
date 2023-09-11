//
// Created by Rahul  Kushwaha on 1/1/23.
//

#include "folly/experimental/TestUtil.h"
#include "log/impl/InMemoryNanoLog.h"
#include "log/impl/NanoLogFactory.h"
#include "log/impl/RocksNanoLog.h"
#include "persistence/RocksDbFactory.h"
#include "persistence/RocksKVStoreLite.h"
#include <gtest/gtest.h>
#include <random>

namespace rk::projects::durable_log {

class NanoLogTests : public testing::TestWithParam<NanoLogType> {
protected:
  std::shared_ptr<folly::test::TemporaryDirectory> tmpDir_;
  std::shared_ptr<rocksdb::DB> rocks_;
  persistence::RocksDbFactory::RocksDbConfig config_{.createIfMissing = true};
  NanoLogType nanoLogType_;

protected:
  void SetUp() override {
    tmpDir_ = std::make_shared<folly::test::TemporaryDirectory>();
    config_.path = tmpDir_->path().string();
    nanoLogType_ = GetParam();
  }
  void TearDown() override {
    if (rocks_) {
      auto s = rocksdb::DestroyDB(config_.path, rocksdb::Options{});
      LOG(INFO) << "db destroy: " << config_.path << ", " << s.ToString();
    }
  }

  std::shared_ptr<NanoLog> makeLog(std::string id, std::string name,
                                   std::string metadataVersionId,
                                   LogId startIndex, LogId endIndex,
                                   bool sealed) {
    switch (nanoLogType_) {
    case NanoLogType::InMemory:
      return std::make_shared<InMemoryNanoLog>(std::move(id), std::move(name),
                                               std::move(metadataVersionId),
                                               startIndex, endIndex, sealed);
    case NanoLogType::Rocks:
      rocks_ = persistence::RocksDbFactory::provideSharedPtr(config_);
      auto kvStore = std::make_shared<persistence::RocksKVStoreLite>(rocks_);
      return std::make_shared<RocksNanoLog>(
          std::move(id), std::move(name), std::move(metadataVersionId),
          startIndex, endIndex, sealed, std::move(kvStore));
    }

    throw std::runtime_error{"unknown nanolog type"};
  }
};

TEST_P(NanoLogTests, AppendWithoutSealDonotFail) {
  auto log = makeLog("id", "name", "versionId", 4, 40, false);

  ASSERT_NO_THROW(log->append({}, 4, "").semi().get()) << "No Exception";
}

TEST_P(NanoLogTests, OutOfOrderAppendNeverFinishes) {
  auto log = makeLog("id", "name", "versionId", 4, 40, false);
  auto future =
      log->append({}, 5, "").semi().via(&folly::InlineExecutor::instance());

  ASSERT_FALSE(future.isReady());
}

TEST_P(NanoLogTests, OrderOfOrderAppendSucceedsAfterAllPreviousAreCompleted) {
  auto log = makeLog("id", "name", "versionId", 4, 40, false);

  auto future6 =
      log->append({}, 6, "").semi().via(&folly::InlineExecutor::instance());
  ASSERT_FALSE(future6.isReady());

  auto future5 =
      log->append({}, 5, "").semi().via(&folly::InlineExecutor::instance());
  ASSERT_FALSE(future6.isReady());
  ASSERT_FALSE(future5.isReady());

  auto future4 =
      log->append({}, 4, "").semi().via(&folly::InlineExecutor::instance());
  ASSERT_EQ(future4.value(), 4);
  ASSERT_EQ(future5.value(), 5);
  ASSERT_EQ(future6.value(), 6);
}

TEST_P(NanoLogTests, MultipleUnorderedAppends) {
  LogId startLogId = 4;
  LogId endLogId = 40;

  auto log = makeLog("id", "name", "versionId", startLogId, endLogId, false);

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
  for (auto logId : logIds) {
    auto future = log->append({}, logId, "")
                      .semi()
                      .via(&folly::InlineExecutor::instance());

    futures.emplace(logId, std::move(future));
  }

  for (auto &[logId, future] : futures) {
    if (logId < pivot) {
      ASSERT_EQ(future.value(), logId);
    } else {
      ASSERT_THROW(future.value(), folly::FutureNotReady) << logId;
    }
  }

  ASSERT_EQ(log->getLocalCommitIndex().semi().get(), pivot);
}

TEST_P(NanoLogTests, FailedAppendsAfterSeal) {
  auto log = makeLog("id", "name", "versionId", 4, 40, false);
  log->seal().semi().get();

  ASSERT_THROW(log->append({}, 4, "").semi().get(), NanoLogSealedException)
      << "Exception Thrown";
}

INSTANTIATE_TEST_SUITE_P(NanoLogParameterizedTests, NanoLogTests,
                         testing::Values(NanoLogType::InMemory,
                                         NanoLogType::Rocks));

} // namespace rk::projects::durable_log
