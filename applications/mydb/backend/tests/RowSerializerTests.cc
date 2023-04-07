//
// Created by Rahul  Kushwaha on 4/7/23.
//
#include <gtest/gtest.h>
#include "applications/mydb/backend/AddTableRowRequest.h"
#include "applications/mydb/backend/RowSerializer.h"
#include "applications/mydb/backend/tests/TestUtils.h"

namespace rk::projects::mydb {
using namespace test_utils;

namespace {
TableSchema createTableSchema() {
  internal::Database database{};
  database.set_name("test");
  database.set_id(1);

  internal::Table table{};
  table.mutable_db()->CopyFrom(database);
  table.set_name("test_table");
  table.set_id(1);

  internal::PrimaryKeyIndex primaryKeyIndex{};
  primaryKeyIndex.set_id(1);
  primaryKeyIndex.mutable_column_ids()->Add(1);

  for (std::int32_t i = 1; i <= 5; i++) {
    Column column{};
    column.set_id(i);
    column.set_name("col" + std::to_string(i));
    table.mutable_columns()->Add(std::move(column));
  }

  table.mutable_primary_key_index()->CopyFrom(primaryKeyIndex);

  std::shared_ptr<Table> tablePtr = std::make_shared<Table>(table);

  TableSchema tableSchema{tablePtr};

  return tableSchema;
}

AddTableRowRequest createAddTableRowRequest() {
  std::shared_ptr<TableSchema>
      tableSchema = std::make_shared<TableSchema>(createTableSchema());
  client::AddRowRequest addRowRequest{};
  addRowRequest.mutable_database_name()->set_name(tableSchema->rawTable().db().name());
  addRowRequest.mutable_table_name()->set_name(tableSchema->rawTable().name());

  for (const auto &column: tableSchema->rawTable().columns()) {
    client::ColumnValue columnValue{};
    columnValue.mutable_name()->set_name(column.name());
    columnValue.mutable_value()->set_value("RANDOM_" + column.name());
    columnValue.mutable_value()->set_type_url("std::string");

    addRowRequest.mutable_column_values()->Add(std::move(columnValue));
  }

  std::unordered_map<std::uint32_t, client::ColumnValue>
      clientColumnValuesLookup;
  for (auto &clientColumnNameValue: *addRowRequest.mutable_column_values()) {
    auto colId = tableSchema->getColumnId(clientColumnNameValue.name().name());

    clientColumnValuesLookup.emplace(colId, clientColumnNameValue);
  }

  AddTableRowRequest request
      {tableSchema, std::move(clientColumnValuesLookup),
       std::move(addRowRequest)};

  return request;
}

}

TEST(RowSerializerTest, serializeRow) {
  auto request = createAddTableRowRequest();
  auto rows = RowSerializer::serialize(request);

  for (const auto &keyValue: rows.keyValues) {
    TableSchemaOutput tableSchema = parse(keyValue.first);
    ASSERT_EQ(request.table->rawTable().db().id(), tableSchema.dbId);
    ASSERT_EQ(request.table->rawTable().id(), tableSchema.tableId);
  }
}

}