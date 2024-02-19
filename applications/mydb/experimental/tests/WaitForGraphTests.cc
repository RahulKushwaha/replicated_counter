//
// Created by Rahul  Kushwaha on 2/18/24.
//

#include "applications/mydb/experimental/Dfs.h"
#include "applications/mydb/experimental/WaitForGraph.h"
#include "fmt/format.h"

#include <gtest/gtest.h>

namespace rk::projects::mydb::experimental::tests {

class WaitForGraphTestFixture : public ::testing::Test {
 protected:
  WaitForGraphTestFixture() = default;

  void SetUp() override {
    resourceManager_ = std::make_shared<ResourceManager>();
    txnManager_ = std::make_shared<TransactionManager>();
  }

 protected:
  std::shared_ptr<ResourceManager> resourceManager_;
  std::shared_ptr<TransactionManager> txnManager_;
};

TEST_F(WaitForGraphTestFixture, addSingleTransaction) {
  auto txn = std::make_shared<Transaction>(Transaction{.id = "1"});

  auto result = txnManager_->add(txn);
  ASSERT_TRUE(result);

  WaitForGraph wfg{resourceManager_, txnManager_};
  wfg.add(txn, {{.name = "A", .lockModeRequired = LockMode::WRITE}});

  Dfs dfs{resourceManager_, txnManager_, txn};
  auto cycle = dfs.getCycle();
  ASSERT_EQ(cycle.size(), 0);
}

TEST_F(WaitForGraphTestFixture, multipleTransactionWithDistinctResources) {
  for (std::int32_t iteration = 0; iteration < 100; iteration++) {
    auto txn = std::make_shared<Transaction>(
        Transaction{.id = fmt::format("{}", iteration)});

    auto result = txnManager_->add(txn);
    ASSERT_TRUE(result);

    WaitForGraph wfg{resourceManager_, txnManager_};
    wfg.add(txn, {{.name = fmt::format("A_{}", iteration),
                   .lockModeRequired = LockMode::WRITE}});

    Dfs dfs{resourceManager_, txnManager_, txn};
    auto cycle = dfs.getCycle();
    ASSERT_EQ(cycle.size(), 0);
  }
}

TEST_F(WaitForGraphTestFixture, multipleTransactionWithSameResource) {
  for (std::int32_t iteration = 0; iteration < 100; iteration++) {
    auto txn = std::make_shared<Transaction>(
        Transaction{.id = fmt::format("{}", iteration)});

    auto result = txnManager_->add(txn);
    ASSERT_TRUE(result);

    WaitForGraph wfg{resourceManager_, txnManager_};
    wfg.add(txn, {{.name = fmt::format("A", iteration),
                   .lockModeRequired = LockMode::WRITE}});

    Dfs dfs{resourceManager_, txnManager_, txn};
    auto cycle = dfs.getCycle();
    ASSERT_EQ(cycle.size(), 0);
  }
}

TEST_F(WaitForGraphTestFixture, twoTransactionsWithCycle) {
  auto txn1 = std::make_shared<Transaction>(Transaction{.id = "1"});
  auto txn2 = std::make_shared<Transaction>(Transaction{.id = "2"});

  auto result = txnManager_->add(txn1);
  assert(result && "txn creation failed");

  result = txnManager_->add(txn2);
  assert(result && "txn creation failed");

  WaitForGraph wfg{resourceManager_, txnManager_};
  wfg.add(txn1, {{.name = "A", .lockModeRequired = LockMode::WRITE}});

  wfg.add(txn2, {{.name = "B", .lockModeRequired = LockMode::WRITE}});

  wfg.add(txn1, {{.name = "B", .lockModeRequired = LockMode::WRITE}});

  wfg.add(txn2, {{.name = "A", .lockModeRequired = LockMode::WRITE}});

  Dfs dfs{resourceManager_, txnManager_, txn1};
  auto cycle = dfs.getCycle();
  ASSERT_EQ("1 -> 2 -> 1", dfs.cycleDebugString());
}

}  // namespace rk::projects::mydb::experimental::tests