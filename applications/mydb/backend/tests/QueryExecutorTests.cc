//
// Created by Rahul  Kushwaha on 4/15/23.
//
#include "applications/mydb/backend/QueryExecutor.h"
#include "applications/mydb/backend/RocksReaderWriter.h"
#include "applications/mydb/backend/RowSerializer.h"
#include "applications/mydb/backend/tests/TestUtils.h"
#include "persistence/tests/RocksTestFixture.h"
#include <gtest/gtest.h>

namespace rk::projects::mydb {

class QueryExecutorTests : public persistence::RocksTestFixture {
protected:
  QueryExecutorTests() {
    queryExecutor_ = std::make_unique<QueryExecutor>(
        std::make_unique<RocksReaderWriter>(db_));
  }

protected:
  std::unique_ptr<QueryExecutor> queryExecutor_;
};

TEST_F(QueryExecutorTests, scanTableUsingPrimaryIndex) {
  auto internalTable = test_utils::getInternalTable(10, 5, 5, 3, 2, 3);

  queryExecutor_->insert(internalTable, InsertOptions{InsertOptions::REPLACE});

  auto response = queryExecutor_->tableScan(
      InternalTable{.schema = internalTable.schema},
      IndexQueryOptions{
          .indexId = internalTable.schema->rawTable().primary_key_index().id(),
          .direction = ScanDirection::FORWARD,
          .maxRowsReturnSize = 100,
      });

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
                          .direction = ScanDirection::FORWARD,
                          .maxRowsReturnSize = 100});

    LOG(INFO) << response.table->ToString() << std::endl;
    LOG(INFO) << internalTable.table->ToString() << std::endl;

    ASSERT_TRUE(internalTable.table->Equals(*response.table));
  }
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
    IndexQueryOptions queryOptions{
        .indexId = internalTable.schema->rawTable().primary_key_index().id(),
        .direction = ScanDirection::FORWARD,
        .primaryKeyValues = primaryKeyValues,
        .maxRowsReturnSize = 2,
    };

    auto response = queryExecutor_->tableScan(
        InternalTable{.schema = internalTable.schema}, queryOptions);

    auto slice = internalTable.table->Slice(startOffset, batchSize);
    LOG(INFO) << response.table->ToString();

    ASSERT_TRUE(slice->Equals(*response.table));

    LOG(INFO) << "Iteration: " << i << " completed";

    auto lastRow = response.table->Slice(batchSize - 1, 1);
    auto keyValues =
        RowSerializer::serialize(InternalTable{internalTable.schema, lastRow});
    ASSERT_EQ(keyValues.size(), 1);
    primaryKeyValues =
        test_utils::parsePrimaryKeyValues({internalTable.schema, lastRow});
  }
}

TEST_F(QueryExecutorTests, scanTableUsingSecondaryIndexWithBatchSize) {
  auto totalRows = 10;
  auto batchSize = 2;
  auto internalTable = test_utils::getInternalTable(totalRows, 5, 5, 1, 1, 1);

  queryExecutor_->insert(internalTable, InsertOptions{});

  std::vector<ColumnValue> secondaryKeyValues;
  std::vector<ColumnValue> primaryKeyValues;

  int i = 0;
  auto firstSecondaryIndex =
      internalTable.schema->rawTable().secondary_index().Get(0);

  while (i < totalRows / batchSize) {
    auto startOffset = i * batchSize;

    IndexQueryOptions queryOptions{
        .indexId = firstSecondaryIndex.id(),
        .direction = ScanDirection::FORWARD,
        .secondaryKeyValues = secondaryKeyValues,
        .primaryKeyValues = primaryKeyValues,
        .maxRowsReturnSize = batchSize,
    };

    auto response = queryExecutor_->tableScan(
        InternalTable{.schema = internalTable.schema}, queryOptions);

    auto slice = internalTable.table->Slice(startOffset, batchSize);
    LOG(INFO) << i;

    LOG(INFO) << response.table->ToString();
    LOG(INFO) << slice->ToString();

    ASSERT_TRUE(slice->Equals(*response.table));

    LOG(INFO) << "Iteration: " << i << " completed";

    auto lastRow = response.table->Slice(batchSize - 1, 1);
    auto keyValues =
        RowSerializer::serialize(InternalTable{internalTable.schema, lastRow});
    ASSERT_EQ(keyValues.size(), 1);

    secondaryKeyValues = test_utils::parseSecondaryKeyValues(
        {internalTable.schema, lastRow}, firstSecondaryIndex.id());
    primaryKeyValues =
        test_utils::parsePrimaryKeyValues({internalTable.schema, lastRow});
    i++;
  }
}

} // namespace rk::projects::mydb