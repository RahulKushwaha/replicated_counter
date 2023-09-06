//
// Created by Aman Mahajan on 4/13/23.
//

#include "applications/counter/CounterApp.h"
#include "applications/counter/CounterAppStateMachine.h"
#include "applications/counter/CounterApplicator.h"
#include "applications/counter/test/InMemoryFakeVirtualLog.h"
#include "folly/experimental/TestUtil.h"
#include "persistence/RocksDbFactory.h"
#include "persistence/RocksKVStoreLite.h"
#include "statemachine/Factory.h"
#include <gtest/gtest.h>

namespace rk::projects::counter_app {
using namespace testing;

class CounterAppTests : public ::testing::Test {
protected:
  std::shared_ptr<folly::test::TemporaryDirectory> rocksDir_;

  void SetUp() override {
    rocksDir_ = std::make_shared<folly::test::TemporaryDirectory>();
  }

  void TearDown() override {}

public:
  std::shared_ptr<CounterApp>
  makeCounterApp(std::optional<std::string> dbPath = {}) {
    auto mockVirtualLog = std::make_shared<InMemoryFakeVirtualLog>();
    auto stateMachineStack = state_machine::makeStateMachineStack<ReturnType>(
        nullptr, mockVirtualLog);

    auto appStateMachine =
        std::make_shared<CounterAppStateMachine>(stateMachineStack);

    persistence::RocksDbFactory::RocksDbConfig config_{};
    if (dbPath.has_value()) {
      config_.path = dbPath.value();
      config_.createIfMissing = false;
      config_.destroyIfExists = false;
    } else {
      config_.path = rocksDir_->path().string();
      config_.createIfMissing = true;
      config_.destroyIfExists = true;
    }

    auto rocks = persistence::RocksDbFactory::provideSharedPtr(config_);
    auto kvStore =
        std::make_shared<persistence::RocksKVStoreLite>(std::move(rocks));

    auto counterApp =
        std::make_shared<CounterApp>(appStateMachine, std::move(kvStore));
    auto applicator = std::make_shared<CounterApplicator>(counterApp);
    appStateMachine->setApplicator(std::move(applicator));
    stateMachineStack->setUpstreamStateMachine(appStateMachine);
    return counterApp;
  }
};

TEST_F(CounterAppTests, successWithSingleKey) {
  auto counterApp = makeCounterApp();

  CounterApp::IncrOperation op1;
  op1.key = "testKey";
  op1.incrBy = 5;

  CounterApp::IncrOperation op2;
  op2.key = "testKey";
  op2.incrBy = 8;

  CounterApp::DecrOperation op3;
  op3.key = "testKey";
  op3.decrBy = 8;

  std::vector<CounterApp::Operation> operations;
  operations.emplace_back(op1);
  operations.emplace_back(op2);
  operations.emplace_back(op3);

  auto res = counterApp->batchUpdate(operations).semi().get();

  ASSERT_EQ(res[0].key, "testKey");
  ASSERT_EQ(res[0].val, 5);
  ASSERT_EQ(res[1].key, "testKey");
  ASSERT_EQ(res[1].val, 13);
  ASSERT_EQ(res[2].key, "testKey");
  ASSERT_EQ(res[2].val, 5);
}

TEST_F(CounterAppTests, successWithMultipleKey) {
  auto counterApp = makeCounterApp();

  CounterApp::IncrOperation op1;
  op1.key = "testKey";
  op1.incrBy = 5;

  CounterApp::IncrOperation op2;
  op2.key = "testKey";
  op2.incrBy = 8;

  CounterApp::DecrOperation op3;
  op3.key = "testKey";
  op3.decrBy = 8;

  CounterApp::IncrOperation op4;
  op4.key = "testKey2";
  op4.incrBy = 20;

  CounterApp::DecrOperation op5;
  op5.key = "testKey2";
  op5.decrBy = 8;

  CounterApp::DecrOperation op6;
  op6.key = "testKey2";
  op6.decrBy = 8;

  std::vector<CounterApp::Operation> operations;
  operations.emplace_back(op1);
  operations.emplace_back(op2);
  operations.emplace_back(op3);
  operations.emplace_back(op4);
  operations.emplace_back(op5);
  operations.emplace_back(op6);

  auto res = counterApp->batchUpdate(operations).semi().get();

  ASSERT_EQ(res[0].key, "testKey");
  ASSERT_EQ(res[0].val, 5);
  ASSERT_EQ(res[1].key, "testKey");
  ASSERT_EQ(res[1].val, 13);
  ASSERT_EQ(res[2].key, "testKey");
  ASSERT_EQ(res[2].val, 5);
  ASSERT_EQ(res[3].key, "testKey2");
  ASSERT_EQ(res[3].val, 20);
  ASSERT_EQ(res[4].key, "testKey2");
  ASSERT_EQ(res[4].val, 12);
  ASSERT_EQ(res[5].key, "testKey2");
  ASSERT_EQ(res[5].val, 4);
}

TEST_F(CounterAppTests, successWithNoOps) {
  auto counterApp = makeCounterApp();

  std::vector<CounterApp::Operation> operations;
  auto res = counterApp->batchUpdate(operations).semi().get();

  ASSERT_EQ(res.size(), 0);
}

TEST_F(CounterAppTests, successWithSingleKeyAndSingleIncrOperation) {
  auto counterApp = makeCounterApp();

  CounterApp::IncrOperation op1;
  op1.key = "testKey";
  op1.incrBy = 5;
  std::vector<CounterApp::Operation> operations;
  operations.emplace_back(op1);

  auto res = counterApp->batchUpdate(operations).semi().get();
  ASSERT_EQ(res[0].key, "testKey");
  ASSERT_EQ(res[0].val, 5);
}

TEST_F(CounterAppTests, successWithSingleKeyAndHundredOperations) {
  auto counterApp = makeCounterApp();

  std::vector<CounterApp::Operation> operations;
  CounterApp::IncrOperation op1;
  op1.key = "testKey";
  op1.incrBy = 1;
  operations.emplace_back(op1);

  for (int i = 0; i <= 99; i++) {
    counterApp->batchUpdate(operations).semi().get();
  }
  auto res = counterApp->batchUpdate(operations).semi().get();
  ASSERT_EQ(res[0].key, "testKey");
  ASSERT_EQ(res[0].val, 101);
}

TEST_F(CounterAppTests, CheckpointEmpty) {
  auto tmpDir = folly::test::TemporaryDirectory("", "/tmp/");
  auto counterApp = makeCounterApp();
  ASSERT_NO_THROW(counterApp->snapshot(tmpDir.path().string()).semi().get());
}

TEST_F(CounterAppTests, CheckpointEmptyAndRestore) {
  auto tmpDir = folly::test::TemporaryDirectory("", "/tmp/");
  {
    auto counterApp = makeCounterApp();
    ASSERT_NO_THROW(counterApp->snapshot(tmpDir.path().string()).semi().get());
    ASSERT_TRUE(counterApp->getValues().empty());
  }

  {
    auto counterApp =
        makeCounterApp(fmt::format("{}/{}", tmpDir.path().string(), "0"));
    auto counterAppSnapshot = counterApp->restoreFromSnapshot().semi().get();
    ASSERT_TRUE(counterApp->getValues().empty());
  }
}

TEST_F(CounterAppTests, CheckpointWithDataAndRestore) {
  auto tmpDir = folly::test::TemporaryDirectory("", "/tmp/");

  // Add some entries and create a checkpoint.
  {
    auto counterApp = makeCounterApp();

    for (int i = 0; i < 100; i++) {
      auto key = fmt::format("testKey_{}", i);
      CounterApp::IncrOperation op1;
      op1.key = key;
      op1.incrBy = 5;
      std::vector<CounterApp::Operation> operations;
      operations.emplace_back(op1);

      auto res = counterApp->batchUpdate(operations).semi().get();
      ASSERT_EQ(res[0].key, key);
      ASSERT_EQ(res[0].val, 5);
    }

    ASSERT_NO_THROW(counterApp->snapshot(tmpDir.path().string()).semi().get());
  }

  // Recreate the app using the checkpoint and check if the data is still
  // present.
  {
    auto counterApp =
        makeCounterApp(fmt::format("{}/{}", tmpDir.path().string(), "0"));
    auto counterAppSnapshot = counterApp->restoreFromSnapshot().semi().get();

    for (int i = 0; i < 100; i++) {
      auto key = fmt::format("testKey_{}", i);
      auto res = counterApp->getValue(key).semi().get();
      ASSERT_EQ(res, 5);
    }

    auto key = fmt::format("testKey_{}", 101);
    CounterApp::IncrOperation op1;
    op1.key = key;
    op1.incrBy = 5;
    std::vector<CounterApp::Operation> operations;
    operations.emplace_back(op1);

    auto res = counterApp->batchUpdate(operations).semi().get();
    ASSERT_EQ(res[0].key, key);
    ASSERT_EQ(res[0].val, 5);
  }
}

TEST_F(CounterAppTests, CheckpointWithDataAndRestoreMultiple) {
  std::vector<std::shared_ptr<folly::test::TemporaryDirectory>> tmpDirs;
  // Add some entries and create a checkpoint.
  {
    for (int index = 0; index < 10; index++) {

      auto counterApp =
          index == 0 ? makeCounterApp()
                     : makeCounterApp(fmt::format(
                           "{}/{}", tmpDirs.back()->path().string(), "0"));

      auto counterAppSnapshot = counterApp->restoreFromSnapshot().semi().get();

      for (int i = 0; i < 100; i++) {
        auto key = fmt::format("testKey_{}", i);
        CounterApp::IncrOperation op1;
        op1.key = key;
        op1.incrBy = 5;
        std::vector<CounterApp::Operation> operations;
        operations.emplace_back(op1);

        auto res = counterApp->batchUpdate(operations).semi().get();
        ASSERT_EQ(res[0].key, key);
        ASSERT_EQ(res[0].val, (index + 1) * 5);
      }

      auto tmpDir =
          std::make_shared<folly::test::TemporaryDirectory>("", "/tmp/");
      tmpDirs.emplace_back(tmpDir);

      auto snapshot =
          counterApp->snapshot(tmpDir->path().string()).semi().get();
    }
  }
}

} // namespace rk::projects::counter_app
