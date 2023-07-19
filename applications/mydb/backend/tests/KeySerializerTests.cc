//
// Created by Rahul  Kushwaha on 4/7/23.
//
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>

#include "applications/mydb/backend/KeySerializer.h"
#include "applications/mydb/backend/proto/db.pb.h"
#include "applications/mydb/backend/tests/TestUtils.h"
#include "google/protobuf/text_format.h"

namespace rk::projects::mydb {
using namespace test_utils;

internal::Table getMetaTables() {
  std::ifstream file(
      "/Users/rahulkushwaha/projects/replicated_counter/applications/mydb/"
      "backend/proto/meta_db_schema.textproto");

  std::ostringstream ss;
  ss << file.rdbuf();
  std::string fileContents = ss.str();
  internal::Table table{};
  auto result =
      google::protobuf::TextFormat::ParseFromString(fileContents, &table);

  if (!result) {
    throw std::runtime_error{"db schemas could not be parsed"};
  }

  return table;
}

class PrefixTestsSuite : public testing::TestWithParam<internal::Table> {};

TEST_P(PrefixTestsSuite, primaryKey) {
  auto table = GetParam();
  auto key = prefix::primaryKey(table, std::vector<ColumnValue>{5000});
  TableSchemaOutput schema = parse(key);

  ASSERT_EQ(schema.dbId, table.db().id());
  ASSERT_EQ(schema.tableId, table.id());
  ASSERT_EQ(schema.indexId, table.primary_key_index().id());
}

TEST_P(PrefixTestsSuite, secondaryIndexKey) {
  auto table = GetParam();
  for (const auto &index : table.secondary_index()) {
    auto key = prefix::secondaryIndexKey(table, index.id(),
                                         std::vector<ColumnValue>{5000},
                                         std::vector<ColumnValue>{5000});
    TableSchemaOutput schema = parse(key);

    ASSERT_EQ(schema.dbId, table.db().id());
    ASSERT_EQ(schema.tableId, table.id());
    ASSERT_EQ(schema.indexId, index.id());
  }
}

TEST_P(PrefixTestsSuite, columnKey) {
  auto table = GetParam();
  for (const auto &column : table.columns()) {
    auto primary = prefix::primaryKey(table, std::vector<ColumnValue>{5000});
    auto key = prefix::columnKey(primary, column.id());

    TableSchemaOutput schema = parse(key);

    ASSERT_EQ(schema.dbId, table.db().id());
    ASSERT_EQ(schema.tableId, table.id());
    ASSERT_EQ(schema.indexId, table.primary_key_index().id());
  }
}

TEST_P(PrefixTestsSuite, parseKeyString) {
  auto table = GetParam();
  auto primary = prefix::primaryKey(table, std::vector<ColumnValue>{5000});
  auto keyFragments = prefix::parseKey(table, primary);

  ASSERT_EQ(keyFragments.dbId, table.db().id());
  ASSERT_EQ(keyFragments.tableId, table.id());
  ASSERT_EQ(keyFragments.primaryIndex->indexId, table.primary_key_index().id());
  ASSERT_EQ(keyFragments.primaryIndex->values.size(), 1);
  ASSERT_EQ(std::stoi(keyFragments.primaryIndex->values[0]),
            table.primary_key_index().column_ids()[0]);
}

INSTANTIATE_TEST_SUITE_P(PrefixTestSuite, PrefixTestsSuite,
                         testing::Values(getMetaTables()));
} // namespace rk::projects::mydb