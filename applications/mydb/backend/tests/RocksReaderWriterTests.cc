//
// Created by Rahul  Kushwaha on 4/10/23.
//

#include "applications/mydb/backend/RocksDbFactory.h"
#include "applications/mydb/backend/RocksReaderWriter.h"
#include "applications/mydb/backend/RowSerializer.h"
#include "applications/mydb/backend/tests/TestUtils.h"
#include "rocksdb/db.h"
#include <gtest/gtest.h>

namespace rk::projects::mydb {

class RocksReaderWriterTests : public ::testing::Test {
protected:
  RocksDbFactory::RocksDbConfig config{.path = "/tmp/db3",
                                       .createIfMissing = true};

  rocksdb::DB *db_;
  std::unique_ptr<RocksReaderWriter> rocksReaderWriter_;

  // You can remove any or all of the following functions if their bodies would
  // be empty.

  RocksReaderWriterTests() {
    // You can do set-up work for each test here.
    db_ = RocksDbFactory::provide(config);
    rocksReaderWriter_ = std::make_unique<RocksReaderWriter>(db_);
  }

  ~RocksReaderWriterTests() override {
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

TEST_F(RocksReaderWriterTests, writeToRockdb) {
  auto internalTable = test_utils::getInternalTable(1);
  auto rawTableRows = RowSerializer::serialize(internalTable);

  bool result = rocksReaderWriter_->write(rawTableRows);
  ASSERT_TRUE(result);

  std::string primaryKey;
  for (auto &[k, v] : rawTableRows[0].keyValues) {
    if (v == "NULL") {
      primaryKey = k;
      break;
    }
  }

  ASSERT_FALSE(primaryKey.empty());

  auto response = rocksReaderWriter_->read({primaryKey});

  ASSERT_FALSE(response.empty());

  auto responseTable =
      RowSerializer::deserialize(internalTable.schema, response);
  ASSERT_TRUE(internalTable.table->Equals(*responseTable.table));
}

TEST_F(RocksReaderWriterTests, writeToRockdbMultipleRows) {
  auto internalTable = test_utils::getInternalTable(10, 5, 5, 3, 2, 3);
  auto rawTableRows = RowSerializer::serialize(internalTable);

  bool result = rocksReaderWriter_->write(rawTableRows);
  ASSERT_TRUE(result);

  std::vector<RawTableRow::Key> keys;
  for (auto &row : rawTableRows) {
    for (auto &[k, v] : row.keyValues) {
      if (v == "NULL") {
        keys.emplace_back(k);
        break;
      }
    }
  }

  ASSERT_EQ(keys.size(), internalTable.table->num_rows());

  auto response = rocksReaderWriter_->read(keys);

  ASSERT_FALSE(response.empty());

  auto responseTable =
      RowSerializer::deserialize(internalTable.schema, response);
  ASSERT_TRUE(internalTable.table->Equals(*responseTable.table));
}

TEST_F(RocksReaderWriterTests, scanTableUsingPrimaryIndex) {
  auto internalTable = test_utils::getInternalTable(10, 5, 5, 3, 2, 3);
  auto rawTableRows = RowSerializer::serialize(internalTable);

  bool result = rocksReaderWriter_->write(rawTableRows);
  ASSERT_TRUE(result);

  auto prefix = prefix::minimumIndexKey(
      internalTable.schema->rawTable(),
      internalTable.schema->rawTable().primary_key_index().id());

  ScanOptions scanOptions{.direction = ScanDirection::FORWARD,
                          .prefix = prefix};
  auto response = rocksReaderWriter_->scan(scanOptions);

  auto responseTable =
      RowSerializer::deserialize(internalTable.schema, response);

  ASSERT_TRUE(internalTable.table->Equals(*responseTable.table));
}

} // namespace rk::projects::mydb