//
// Created by Rahul  Kushwaha on 4/10/23.
//

#include <gtest/gtest.h>
#include "rocksdb/db.h"
#include "applications/mydb/backend/RocksDbFactory.h"
#include "applications/mydb/backend/RocksReaderWriter.h"
#include "applications/mydb/backend/tests/TestUtils.h"
#include "applications/mydb/backend/RowSerializer.h"

namespace rk::projects::mydb {

class RocksReaderWriterTests: public ::testing::Test {
 protected:
  RocksDbFactory::RocksDbConfig config{
      .path = "/tmp/db3",
      .createIfMissing = true
  };

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

  for (auto &[k, v]: rawTableRows[0].keyValues) {
    if (v == "NULL") {
      primaryKey = k;
      break;
    }
  }

  ASSERT_FALSE(primaryKey.empty());

  auto response = rocksReaderWriter_->read({primaryKey});

  ASSERT_FALSE(response.empty());

  for (const auto &row: response) {
    for (const auto &[k, v]: row.keyValues) {
      LOG(INFO) << "key: " << k << " " << "value: " << v;
      auto rawTable = internalTable.schema->rawTable();
      auto keyValueFragments = prefix::parseKey(rawTable, k);

      ASSERT_EQ(keyValueFragments.dbId, rawTable.db().id());
      ASSERT_EQ(keyValueFragments.tableId, rawTable.id());

      if (keyValueFragments.primaryIndex.has_value()) {
        ASSERT_EQ(keyValueFragments.primaryIndex.value().indexId,
                  rawTable.primary_key_index().id());
      }

      if(keyValueFragments.secondaryIndex.has_value()){
      }
    }
  }
}

}