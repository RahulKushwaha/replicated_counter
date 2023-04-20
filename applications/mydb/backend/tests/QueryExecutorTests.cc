//
// Created by Rahul  Kushwaha on 4/15/23.
//
#include <gtest/gtest.h>
#include "rocksdb/db.h"
#include "applications/mydb/backend/RocksDbFactory.h"
#include "applications/mydb/backend/RowSerializer.h"
#include "applications/mydb/backend/RocksReaderWriter.h"
#include "applications/mydb/backend/tests/TestUtils.h"
#include "applications/mydb/backend/QueryExecutor.h"
#include "TestUtils.h"

namespace rk::projects::mydb {

class QueryExecutorTests: public ::testing::Test {
 protected:
  RocksDbFactory::RocksDbConfig config{
      .path = "/tmp/db3",
      .createIfMissing = true
  };

  rocksdb::DB *db_;
  std::unique_ptr<QueryExecutor> queryExecutor_;

  // You can remove any or all of the following functions if their bodies would
  // be empty.

  QueryExecutorTests() {
    // You can do set-up work for each test here.
    db_ = RocksDbFactory::provide(config);
    queryExecutor_ =
        std::make_unique<QueryExecutor>(std::make_unique<RocksReaderWriter>(db_));
  }

  ~QueryExecutorTests() override {
    db_->Close();
    auto status = rocksdb::DestroyDB(config.path, rocksdb::Options{});
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

TEST_F(QueryExecutorTests, getNonExistingRow) {
  auto internalTable = test_utils::getInternalTable(10, 5, 5, 3, 2, 3);
  auto response = queryExecutor_->get(internalTable);

  for (const auto &responseColumn: response.table->columns()) {
    ASSERT_EQ(0, responseColumn->length());
  }
}

TEST_F(QueryExecutorTests, insertAndGet) {
  auto internalTable = test_utils::getInternalTable(10, 5, 5, 3, 2, 3);

  queryExecutor_->insert(internalTable, InsertOptions::REPLACE);
  auto response = queryExecutor_->get(internalTable);

  ASSERT_TRUE(internalTable.table->Equals(*response.table));
}

TEST_F(QueryExecutorTests, insertAndReplace) {
  auto internalTable = test_utils::getInternalTable(10, 5, 5, 3, 2, 3);

  queryExecutor_->insert(internalTable, InsertOptions::REPLACE);
  auto response = queryExecutor_->get(internalTable);

  ASSERT_TRUE(internalTable.table->Equals(*response.table));
}

TEST_F(QueryExecutorTests, scanTableUsingPrimaryIndex) {
  auto internalTable = test_utils::getInternalTable(10, 5, 5, 3, 2, 3);

  queryExecutor_->insert(internalTable, InsertOptions{});
  IndexQueryOptions queryOptions
      {
          .indexId = internalTable.schema->rawTable().primary_key_index().id(),
          .direction = ScanDirection::FORWARD,
          .maxRowsReturnSize = std::numeric_limits<std::int32_t>::max()
      };

  auto response =
      queryExecutor_->tableScan(InternalTable{.schema = internalTable.schema},
                                queryOptions);

  ASSERT_TRUE(internalTable.table->Equals(*response.table));
}

TEST_F(QueryExecutorTests, scanTableUsingPrimaryIndexWithBatchSize) {
  auto totalRows = 10;
  auto batchSize = 2;
  auto internalTable = test_utils::getInternalTable(totalRows, 5, 5, 3, 2, 3);

  queryExecutor_->insert(internalTable, InsertOptions{});

  std::vector<ColumnValue> primaryKeyValues;

  for (auto i = 0; i < totalRows / batchSize; i++) {
    LOG(INFO) << "Iteration: " << i << " started";
    auto startOffset = i * batchSize;
    IndexQueryOptions queryOptions
        {
            .indexId = internalTable.schema->rawTable().primary_key_index().id(),
            .direction = ScanDirection::FORWARD,
            .startFromKey = primaryKeyValues,
            .maxRowsReturnSize = 2,
        };

    auto response =
        queryExecutor_->tableScan(InternalTable{.schema = internalTable.schema},
                                  queryOptions);

    auto slice = internalTable.table->Slice(startOffset, batchSize);
    LOG(INFO) << slice->ToString();
    LOG(INFO) << response.table->ToString();
    ASSERT_TRUE(slice->Equals(*response.table));

    LOG(INFO) << "Iteration: " << i << " completed";

    auto lastRow = response.table->Slice(batchSize - 1, 1);
    auto keyValues =
        RowSerializer::serialize(InternalTable{internalTable.schema,
                                               lastRow});
    ASSERT_EQ(keyValues.size(), 1);
    primaryKeyValues =
        test_utils::parsePrimaryKeyValues({internalTable.schema, lastRow});
  }
}

TEST_F(QueryExecutorTests, scanTableUsingSecondaryIndex) {
  auto internalTable = test_utils::getInternalTable(1, 5, 5, 3, 1, 1);

  queryExecutor_->insert(internalTable, InsertOptions{});

  for (const auto &idx: internalTable.schema->rawTable().secondary_index()) {
    auto response =
        queryExecutor_->tableScan(
            InternalTable{
                .schema = internalTable.schema
            },
            IndexQueryOptions{
                .indexId = idx.id(),
                .direction = ScanDirection::FORWARD,
            });

    LOG(INFO) << response.table->ToString() << std::endl;

    ASSERT_TRUE(internalTable.table->Equals(*response.table));
  }
}

}