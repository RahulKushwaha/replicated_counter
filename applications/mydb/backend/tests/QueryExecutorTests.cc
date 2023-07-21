//
// Created by Rahul  Kushwaha on 4/15/23.
//
#include "applications/mydb/backend/QueryExecutor.h"
#include "applications/mydb/backend/RocksDbFactory.h"
#include "applications/mydb/backend/RocksReaderWriter.h"
#include "applications/mydb/backend/RowSerializer.h"
#include "applications/mydb/backend/tests/TestUtils.h"
#include "rocksdb/db.h"
#include <gtest/gtest.h>

namespace rk::projects::mydb {

class QueryExecutorTests : public ::testing::Test {
protected:
  RocksDbFactory::RocksDbConfig config{.path = "/tmp/db3",
                                       .createIfMissing = true};

  rocksdb::DB *db_;
  std::unique_ptr<QueryExecutor> queryExecutor_;

  // You can remove any or all of the following functions if their bodies would
  // be empty.

  QueryExecutorTests() {
    // You can do set-up work for each test here.
    db_ = RocksDbFactory::provide(config);
    queryExecutor_ = std::make_unique<QueryExecutor>(
        std::make_unique<RocksReaderWriter>(db_));
  }

  ~QueryExecutorTests() override {
    db_->Close();
    auto status = rocksdb::DestroyDB(config.path, rocksdb::Options{});
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
};

TEST_F(QueryExecutorTests, scanTableUsingPrimaryIndex) {
  auto internalTable = test_utils::getInternalTable(10, 5, 5, 3, 2, 3);

  queryExecutor_->insert(internalTable, InsertOptions{InsertOptions::REPLACE});

  auto response = queryExecutor_->tableScan(
      InternalTable{.schema = internalTable.schema},
      IndexQueryOptions{
          .indexId = internalTable.schema->rawTable().primary_key_index().id(),
          .direction = ScanDirection::FORWARD});

  LOG(INFO) << response.table->ToString() << std::endl;
  LOG(INFO) << internalTable.table->ToString() << std::endl;

  ASSERT_TRUE(internalTable.table->Equals(*response.table));
}

TEST_F(QueryExecutorTests, scanTableUsingSecondaryIndex) {
  auto internalTable = test_utils::getInternalTable(1, 0, 5, 1, 1, 1);

  queryExecutor_->insert(internalTable, InsertOptions{InsertOptions::REPLACE});

  for (const auto &idx : internalTable.schema->rawTable().secondary_index()) {
    auto response = queryExecutor_->tableScan(
        InternalTable{.schema = internalTable.schema},
        IndexQueryOptions{.indexId = idx.id(),
                          .direction = ScanDirection::FORWARD});

    LOG(INFO) << response.table->ToString() << std::endl;
    LOG(INFO) << internalTable.table->ToString() << std::endl;

    ASSERT_TRUE(internalTable.table->Equals(*response.table));
  }
}

} // namespace rk::projects::mydb