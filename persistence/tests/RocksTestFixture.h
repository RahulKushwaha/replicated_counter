//
// Created by Rahul  Kushwaha on 9/20/23.
//

#pragma once
#include "folly/experimental/TestUtil.h"
#include "persistence/RocksDbFactory.h"
#include "gtest/gtest.h"

namespace rk::projects::persistence {

class RocksTestFixture : public ::testing::Test {
protected:
  RocksTestFixture()
      : tmpDir_{std::make_shared<folly::test::TemporaryDirectory>()},
        config_{RocksDbFactory::RocksDbConfig{.path = tmpDir_->path().string(),
                                              .createIfMissing = true}},
        optimisticDb_{RocksDbFactory::provideOptimisticDb(config_)},
        db_{optimisticDb_->GetBaseDB()} {}

  ~RocksTestFixture() override {
    db_->Close();
    auto status = rocksdb::DestroyDB(config_.path, rocksdb::Options{});
    LOG(INFO) << status.ToString();
    assert(status.ok());
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  void SetUp() override {
    // Code here will be called immediately after the constructor (right
    // before each test).
  }

  void TearDown() override {
    // Code here will be called immediately after each test (right
    // before the destructor).
  }

protected:
  std::shared_ptr<folly::test::TemporaryDirectory> tmpDir_;
  RocksDbFactory::RocksDbConfig config_;
  std::shared_ptr<rocksdb::OptimisticTransactionDB> optimisticDb_;
  rocksdb::DB *db_;
};

} // namespace rk::projects::persistence