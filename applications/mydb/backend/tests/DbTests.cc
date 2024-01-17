//
// Created by rahul on 12/25/23.
//

#include "applications/mydb/backend/Bootstrap.h"
#include "applications/mydb/backend/Db.h"
#include "applications/mydb/backend/DbDefaults.h"
#include "applications/mydb/backend/Errors.h"
#include "applications/mydb/backend/SchemaStore.h"
#include "google/protobuf/any.pb.h"
#include "persistence/tests/RocksTestFixture.h"

#include <gtest/gtest.h>

namespace rk::projects::mydb {

namespace {

std::vector<std::vector<client::ColumnValue>> getSampleDataRow(
    std::shared_ptr<internal::Table> table, std::int32_t numRows) {
  std::vector<std::vector<client::ColumnValue>> rows;
  std::uint64_t intColValue{0};
  std::string randomValue{"RANDOM_VALUE+"};
  for (std::int32_t index = 0; index < numRows; index++) {
    std::vector<client::ColumnValue> row;

    for (auto& col : table->columns()) {
      client::ColumnValue columnValue{};
      columnValue.mutable_name()->set_name(col.name());
      google::protobuf::Any value{};
      switch (col.column_type()) {
        case Column_COLUMN_TYPE_INT64: {
          if (TableDefaultColumns::LOCK_MODE_COL_ID == col.id()) {
            value.set_value(std::to_string(enumToInteger(LockType::NO_LOCK)));
          } else {
            value.set_value(std::to_string(intColValue++));
          }
        } break;
        case Column_COLUMN_TYPE_STRING: {
          value.set_value(randomValue + std::to_string(intColValue++));
        } break;
        default:
          assert(false && "unknown column type");
      }

      *columnValue.mutable_value() = std::move(value);
      row.emplace_back(std::move(columnValue));
    }

    rows.emplace_back(std::move(row));
  }

  return rows;
}

client::AddTableRequest getAddTableRequest(std::string dbName,
                                           std::string tableName,
                                           std::int32_t numIntColums = 3,
                                           std::int32_t numStringColumns = 3) {
  client::AddTableRequest addTableRequest{};
  addTableRequest.mutable_table()->mutable_db()->set_name(dbName);
  addTableRequest.mutable_table()->mutable_name()->set_name(tableName);

  // add 3 integer columns
  for (std::int32_t i = 0; i < numIntColums; i++) {
    client::Column column{};
    column.mutable_name()->set_name(fmt::format("{}_int_col", i));
    column.set_column_type(client::Column_COLUMN_TYPE_INT64);

    addTableRequest.mutable_table()->mutable_columns()->Add(std::move(column));
  }

  // add 3 string columns
  for (std::int32_t i = 0; i < numStringColumns; i++) {
    client::Column column{};
    column.mutable_name()->set_name(fmt::format("{}_str_col", i));
    column.set_column_type(client::Column_COLUMN_TYPE_STRING);

    addTableRequest.mutable_table()->mutable_columns()->Add(std::move(column));
  }

  client::ColumnName primaryColName1{};
  primaryColName1.set_name("0_int_col");

  client::ColumnName primaryColName2{};
  primaryColName2.set_name("0_str_col");

  addTableRequest.mutable_table()
      ->mutable_primary_key()
      ->mutable_columns()
      ->Add(std::move(primaryColName1));

  addTableRequest.mutable_table()
      ->mutable_primary_key()
      ->mutable_columns()
      ->Add(std::move(primaryColName2));

  return addTableRequest;
}

client::AddDatabaseRequest getAddDatabaseRequest(std::string dbName) {
  client::AddDatabaseRequest addDatabaseRequest;
  addDatabaseRequest.mutable_database()->mutable_name()->set_name(dbName);
  return addDatabaseRequest;
}

}  // namespace

const std::string metaDbSchemaFileLocation =
    "/Users/rahulkushwaha/projects/replicated_counter/applications/mydb/"
    "backend/proto/meta_db_schema.textproto";
const std::string metaDbTableSchemaFileLocation =
    "/Users/rahulkushwaha/projects/replicated_counter/applications/mydb/"
    "backend/proto/meta_db_table_schema.textproto";

class DbTests : public persistence::RocksTestFixture {
 protected:
  DbTests()
      : rocks_{std::make_unique<RocksReaderWriter>(db_)},
        queryExecutor_{std::make_shared<QueryExecutor>(rocks_)} {}

 protected:
  std::shared_ptr<RocksReaderWriter> rocks_;
  std::shared_ptr<QueryExecutor> queryExecutor_;
};

TEST_F(DbTests, addDatabaseTest) {
  auto schemaStore = std::make_shared<SchemaStore>();
  auto result = bootstrap(metaDbSchemaFileLocation,
                          metaDbTableSchemaFileLocation, schemaStore);
  ASSERT_TRUE(result);

  Db db{schemaStore, rocks_};
  auto addDatabaseRequest = getAddDatabaseRequest("temp");
  db.addDatabase(&addDatabaseRequest);

  client::ScanTableRequest scanTableRequest;
  client::ColumnName idColumnName;
  idColumnName.set_name("id");
  scanTableRequest.mutable_return_columns()->Add(std::move(idColumnName));

  scanTableRequest.mutable_database_name()->set_name("meta");
  scanTableRequest.mutable_table_name()->set_name("meta_db");

  auto intCondition = IntCondition{};
  intCondition.set_col_name("id");
  intCondition.set_op(IntCondition_Operation_EQ);
  intCondition.set_value(0);

  auto unaryCondition = UnaryCondition{};
  unaryCondition.mutable_int_condition()->CopyFrom(intCondition);

  scanTableRequest.mutable_condition()->mutable_unary_condition()->CopyFrom(
      unaryCondition);

  auto tableRows = db.scanTable(&scanTableRequest);
  ASSERT_EQ(tableRows.rows().size(), 1);
}

TEST_F(DbTests, addSameDatabaseMultipleTimesThrowsException) {
  auto schemaStore = std::make_shared<SchemaStore>();
  auto result = bootstrap(metaDbSchemaFileLocation,
                          metaDbTableSchemaFileLocation, schemaStore);
  ASSERT_TRUE(result);

  Db db{schemaStore, rocks_};
  auto addDatabaseRequest = getAddDatabaseRequest("temp");
  db.addDatabase(&addDatabaseRequest);

  std::optional<DbError> errorResponse;
  try {
    auto addDatabaseResponse = db.addDatabase(&addDatabaseRequest);
  } catch (DbError& dbError) {
    errorResponse.emplace(std::move(dbError));
  }

  ASSERT_TRUE(errorResponse.has_value());
  ASSERT_EQ(errorResponse.value().errorCode_,
            ErrorCode::DATABASE_ALREADY_EXISTS);
}

TEST_F(DbTests, addTable) {
  auto schemaStore = std::make_shared<SchemaStore>();
  auto result = bootstrap(metaDbSchemaFileLocation,
                          metaDbTableSchemaFileLocation, schemaStore);
  ASSERT_TRUE(result);

  Db db{schemaStore, rocks_};

  auto addDatabaseRequest = getAddDatabaseRequest("temp");
  db.addDatabase(&addDatabaseRequest);

  auto addTableRequest = getAddTableRequest("temp", "tbl_1");
  db.addTable(&addTableRequest);
}

TEST_F(DbTests, addSameTableMultipleTimesThrowsException) {
  auto schemaStore = std::make_shared<SchemaStore>();
  auto result = bootstrap(metaDbSchemaFileLocation,
                          metaDbTableSchemaFileLocation, schemaStore);
  ASSERT_TRUE(result);

  Db db{schemaStore, rocks_};

  auto addDatabaseRequest = getAddDatabaseRequest("temp");
  db.addDatabase(&addDatabaseRequest);

  auto addTableRequest = getAddTableRequest("temp", "tbl_1");
  db.addTable(&addTableRequest);

  std::optional<DbError> errorResponse;
  try {
    auto addTableResponse = db.addTable(&addTableRequest);
  } catch (DbError& dbError) {
    errorResponse.emplace(std::move(dbError));
  }

  ASSERT_TRUE(errorResponse.has_value());
  ASSERT_EQ(errorResponse.value().errorCode_, ErrorCode::TABLE_ALREADY_EXISTS);
}

TEST_F(DbTests, addTableRow) {
  auto schemaStore = std::make_shared<SchemaStore>();
  auto result = bootstrap(metaDbSchemaFileLocation,
                          metaDbTableSchemaFileLocation, schemaStore);
  ASSERT_TRUE(result);

  auto dbName{"temp"};
  auto tableName{"tbl_1"};

  Db db{schemaStore, rocks_};

  auto addDatabaseRequest = getAddDatabaseRequest(dbName);
  db.addDatabase(&addDatabaseRequest);

  auto addTableRequest = getAddTableRequest(dbName, tableName);
  db.addTable(&addTableRequest);

  auto tableSchema = schemaStore->getTable(dbName, tableName);
  ASSERT_TRUE(tableSchema.has_value());
  auto tableSchemaPtr = std::make_shared<internal::Table>(tableSchema.value());

  client::AddRowRequest row{};
  row.mutable_database_name()->set_name(dbName);
  row.mutable_table_name()->set_name(tableName);

  auto rows = getSampleDataRow(tableSchemaPtr, 1);

  row.mutable_column_values()->Add(rows.at(0).begin(), rows.at(0).end());

  auto response = db.addRow(&row);
  ASSERT_TRUE(response.has_table_rows());
  ASSERT_EQ(response.table_rows().rows().size(), 1);
}

TEST_F(DbTests, updateTableRow) {
  auto schemaStore = std::make_shared<SchemaStore>();
  auto result = bootstrap(metaDbSchemaFileLocation,
                          metaDbTableSchemaFileLocation, schemaStore);
  ASSERT_TRUE(result);

  auto dbName{"temp"};
  auto tableName{"tbl_1"};

  Db db{schemaStore, rocks_};

  auto addDatabaseRequest = getAddDatabaseRequest(dbName);
  db.addDatabase(&addDatabaseRequest);

  auto addTableRequest = getAddTableRequest(dbName, tableName);
  db.addTable(&addTableRequest);

  auto tableSchema = schemaStore->getTable(dbName, tableName);
  ASSERT_TRUE(tableSchema.has_value());
  auto tableSchemaPtr = std::make_shared<internal::Table>(tableSchema.value());

  client::AddRowRequest row{};
  row.mutable_database_name()->set_name(dbName);
  row.mutable_table_name()->set_name(tableName);

  auto rows = getSampleDataRow(tableSchemaPtr, 1);
  auto rowData = rows.at(0);

  row.mutable_column_values()->Add(rowData.begin(), rowData.end());

  auto response = db.addRow(&row);
  ASSERT_TRUE(response.has_table_rows());
  ASSERT_EQ(response.table_rows().rows().size(), 1);

  client::UpdateRowRequest updateRow{};
  updateRow.mutable_database_name()->set_name(dbName);
  updateRow.mutable_table_name()->set_name(tableName);

  updateRow.mutable_column_values()->Add(rowData.begin(), rowData.end());

  auto updateResponse = db.updateRow(&updateRow);
}

TEST_F(DbTests, updateTableRowIfNotExists) {
  auto schemaStore = std::make_shared<SchemaStore>();
  auto result = bootstrap(metaDbSchemaFileLocation,
                          metaDbTableSchemaFileLocation, schemaStore);
  ASSERT_TRUE(result);

  auto dbName{"temp"};
  auto tableName{"tbl_1"};

  Db db{schemaStore, rocks_};

  auto addDatabaseRequest = getAddDatabaseRequest(dbName);
  db.addDatabase(&addDatabaseRequest);

  auto addTableRequest = getAddTableRequest(dbName, tableName);
  db.addTable(&addTableRequest);

  auto tableSchema = schemaStore->getTable(dbName, tableName);
  ASSERT_TRUE(tableSchema.has_value());
  auto tableSchemaPtr = std::make_shared<internal::Table>(tableSchema.value());

  auto rows = getSampleDataRow(tableSchemaPtr, 1);
  auto rowData = rows.at(0);

  client::UpdateRowRequest updateRow{};
  updateRow.mutable_database_name()->set_name(dbName);
  updateRow.mutable_table_name()->set_name(tableName);

  updateRow.mutable_column_values()->Add(rowData.begin(), rowData.end());

  std::optional<DbError> errorResponse;
  try {
    auto updateResponse = db.updateRow(&updateRow);
  } catch (DbError& dbError) {
    errorResponse.emplace(std::move(dbError));
  }

  ASSERT_TRUE(errorResponse.has_value());
  ASSERT_EQ(errorResponse.value().errorCode_,
            ErrorCode::UPDATE_CONDITION_FAILED);
}

TEST_F(DbTests, updateTableRowWithCondition) {
  auto schemaStore = std::make_shared<SchemaStore>();
  auto result = bootstrap(metaDbSchemaFileLocation,
                          metaDbTableSchemaFileLocation, schemaStore);
  ASSERT_TRUE(result);

  auto dbName{"temp"};
  auto tableName{"tbl_1"};

  Db db{schemaStore, rocks_};

  auto addDatabaseRequest = getAddDatabaseRequest(dbName);
  db.addDatabase(&addDatabaseRequest);

  auto addTableRequest = getAddTableRequest(dbName, tableName);
  db.addTable(&addTableRequest);

  auto tableSchema = schemaStore->getTable(dbName, tableName);
  ASSERT_TRUE(tableSchema.has_value());
  auto tableSchemaPtr = std::make_shared<internal::Table>(tableSchema.value());

  client::AddRowRequest row{};
  row.mutable_database_name()->set_name(dbName);
  row.mutable_table_name()->set_name(tableName);

  auto rows = getSampleDataRow(tableSchemaPtr, 1);
  auto rowData = rows.at(0);

  row.mutable_column_values()->Add(rowData.begin(), rowData.end());

  auto response = db.addRow(&row);
  ASSERT_TRUE(response.has_table_rows());
  ASSERT_EQ(response.table_rows().rows().size(), 1);

  // Successful where condition
  {
    client::UpdateRowRequest updateRow{};
    updateRow.mutable_database_name()->set_name(dbName);
    updateRow.mutable_table_name()->set_name(tableName);

    updateRow.mutable_column_values()->Add(rowData.begin(), rowData.end());

    Condition condition{};
    auto intCondition =
        condition.mutable_unary_condition()->mutable_int_condition();
    intCondition->set_col_name("2_int_col");
    intCondition->set_value(2);
    intCondition->set_op(IntCondition_Operation_EQ);

    *updateRow.mutable_condition() = std::move(condition);

    auto updateResponse = db.updateRow(&updateRow);
  }

  // Failing where condition
  {
    client::UpdateRowRequest updateRow{};
    updateRow.mutable_database_name()->set_name(dbName);
    updateRow.mutable_table_name()->set_name(tableName);

    updateRow.mutable_column_values()->Add(rowData.begin(), rowData.end());

    Condition condition{};
    auto intCondition =
        condition.mutable_unary_condition()->mutable_int_condition();
    intCondition->set_col_name("2_int_col");
    intCondition->set_value(3);
    intCondition->set_op(IntCondition_Operation_EQ);

    updateRow.mutable_condition()->CopyFrom(condition);

    std::optional<DbError> errorResponse;
    try {
      auto updateResponse = db.updateRow(&updateRow);
    } catch (DbError& dbError) {
      errorResponse.emplace(std::move(dbError));
    }

    ASSERT_TRUE(errorResponse.has_value());
    ASSERT_EQ(errorResponse.value().errorCode_,
              ErrorCode::UPDATE_CONDITION_FAILED);
  }
}

}  // namespace rk::projects::mydb