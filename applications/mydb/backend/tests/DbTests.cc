//
// Created by rahul on 12/25/23.
//

#include "applications/mydb/backend/Bootstrap.h"
#include "applications/mydb/backend/Db.h"
#include "applications/mydb/backend/SchemaStore.h"
#include "persistence/tests/RocksTestFixture.h"

#include <boost/random/detail/signed_unsigned_tools.hpp>
#include <gtest/gtest.h>

namespace rk::projects::mydb {

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
  client::AddDatabaseRequest addDatabaseRequest;
  addDatabaseRequest.mutable_database()->mutable_name()->set_name("temp");

  db.addDatabase(&addDatabaseRequest);

  client::ScanTableRequest scanTableRequest;
  client::ColumnName idColumnName;
  idColumnName.set_name("id");
  scanTableRequest.mutable_return_columns()->Add(std::move(idColumnName));

  scanTableRequest.mutable_database_name()->set_name("meta");
  scanTableRequest.mutable_table_name()->set_name("meta_db");

  auto intCondtion = client::IntCondition{};
  intCondtion.set_col_name("id");
  intCondtion.set_op(client::IntCondition_Operation_EQ);
  intCondtion.set_value(0);

  auto unaryCondition = client::UnaryCondition{};
  unaryCondition.mutable_int_condition()->CopyFrom(intCondtion);

  scanTableRequest.mutable_condition()->mutable_unary_condition()->CopyFrom(
      unaryCondition);

  auto tableRows = db.scanDatabase(&scanTableRequest);
  ASSERT_EQ(tableRows.rows().size(), 1);
}

TEST_F(DbTests, addSameDatabaseMultipleTimesThrowsException) {
  auto schemaStore = std::make_shared<SchemaStore>();
  auto result = bootstrap(metaDbSchemaFileLocation,
                          metaDbTableSchemaFileLocation, schemaStore);
  ASSERT_TRUE(result);

  Db db{schemaStore, rocks_};
  client::AddDatabaseRequest addDatabaseRequest;
  addDatabaseRequest.mutable_database()->mutable_name()->set_name("temp");

  db.addDatabase(&addDatabaseRequest);

  ASSERT_THROW(db.addDatabase(&addDatabaseRequest), std::runtime_error);
}

TEST_F(DbTests, addTable) {
  auto schemaStore = std::make_shared<SchemaStore>();
  auto result = bootstrap(metaDbSchemaFileLocation,
                          metaDbTableSchemaFileLocation, schemaStore);
  ASSERT_TRUE(result);

  Db db{schemaStore, rocks_};
  client::AddDatabaseRequest addDatabaseRequest;
  addDatabaseRequest.mutable_database()->mutable_name()->set_name("temp");

  db.addDatabase(&addDatabaseRequest);

  client::AddTableRequest addTableRequest{};
  addTableRequest.mutable_database()->mutable_name()->set_name("temp");
  addTableRequest.mutable_table()->mutable_name()->set_name("tbl_1");

  db.addTable(&addTableRequest);
}

}  // namespace rk::projects::mydb