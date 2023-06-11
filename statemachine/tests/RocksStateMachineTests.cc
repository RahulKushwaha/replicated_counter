//
// Created by Rahul  Kushwaha on 6/10/23.
//

#include "applications/mydb/backend/RocksDbFactory.h"
#include "statemachine/RocksStateMachine.h"
#include "statemachine/RocksTxnApplicator.h"
#include "statemachine/tests/TestUtils.h"
#include "wor/WriteOnceRegisterChainAppender.h"
#include "wor/inmemory/InMemoryWriteOnceRegisterChain.h"
#include <fmt/format.h>
#include <gtest/gtest.h>

namespace rk::projects::state_machine {
using namespace wor;
using namespace rk::projects::mydb;

class RocksStateMachineTests : public ::testing::Test {
protected:
  RocksDbFactory::RocksDbConfig config{.path = "/tmp/db3",
                                       .createIfMissing = true};

  rocksdb::DB *db_;

  RocksStateMachineTests() {
    // You can do set-up work for each test here.
    db_ = RocksDbFactory::provide(RocksDbFactory::RocksDbConfig{
        .path = "/tmp/db3", .createIfMissing = true});
  }

  ~RocksStateMachineTests() override {
    db_->Close();
    auto status = rocksdb::DestroyDB(config.path, rocksdb::Options{});
    LOG(INFO) << status.ToString();
    assert(status.ok());
  }

  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(RocksStateMachineTests, WriteSingleTxn) {
  using appender_t = wor::WriteOnceRegisterChainAppender<std::string>;
  auto conflictDetector = std::make_shared<ConflictDetector>();
  auto applicator = std::make_shared<RocksTxnApplicator>(conflictDetector, db_);
  auto chain = std::make_shared<InMemoryWriteOnceRegisterChain>();
  RocksStateMachine stm{applicator, chain};
  stm.setApplicator(applicator);

  auto txn = std::move(*TestUtils::createNonConflictingTxns(1).begin());
  auto txnResult = stm.append(std::move(txn)).semi().get();

  ASSERT_TRUE(txnResult.txnSucceeded);
  ASSERT_FALSE(txnResult.speculativeTxnResult.txnSucceeded);
  ASSERT_FALSE(txnResult.speculativeTxnResult.conflictDetected);
}

} // namespace rk::projects::state_machine