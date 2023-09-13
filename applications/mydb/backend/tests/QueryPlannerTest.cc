//
// Created by Aman Mahajan on 9/10/23.
//

#include "applications/mydb/backend/QueryPlanner.h"
#include "TestUtils.h"
#include "applications/mydb/backend/QueryExecutor.h"
#include "applications/mydb/backend/RocksDbFactory.h"
#include <gtest/gtest.h>
namespace rk::projects::mydb {

class QueryPlannerTests : public ::testing::Test {
protected:
  RocksDbFactory::RocksDbConfig config{.path = "/tmp/db3",
                                       .createIfMissing = true};

  rocksdb::DB *db_;
  std::shared_ptr<QueryExecutor> queryExecutor_;

  // You can remove any or all of the following functions if their bodies would
  // be empty.

  QueryPlannerTests() {}

  ~QueryPlannerTests() override {}

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  void SetUp() override {
    // You can do set-up work for each test here.
    db_ = RocksDbFactory::provide(config);
    queryExecutor_ = std::make_shared<QueryExecutor>(
        std::make_unique<RocksReaderWriter>(db_));
    // Code here will be called immediately after the constructor (right
    // before each test).
  }

  void TearDown() override {
    db_->Close();
    auto status = rocksdb::DestroyDB(config.path, rocksdb::Options{});
    assert(status.ok());
    // Code here will be called immediately after each test (right
    // before the destructor).
  }
  void testIntUnaryCondition(client::IntCondition_Operation operation,
                             int64_t value, int64_t returnedRows,
                             const InternalTable &internalTable);
};

TEST_F(QueryPlannerTests, scanTableUsingPrimaryIndexWithIntUnaryCondition) {
  auto internalTable = test_utils::getInternalTable(10, 5, 0, 1, 1, 1);
  queryExecutor_->insert(internalTable, InsertOptions{InsertOptions::REPLACE});
  testIntUnaryCondition(client::IntCondition_Operation_GT, 5, 4, internalTable);
  testIntUnaryCondition(client::IntCondition_Operation_GEQ, 5, 5,
                        internalTable);
  testIntUnaryCondition(client::IntCondition_Operation_LT, 6, 6, internalTable);
  testIntUnaryCondition(client::IntCondition_Operation_LEQ, 6, 7,
                        internalTable);
  testIntUnaryCondition(client::IntCondition_Operation_EQ, 5, 1, internalTable);
}

void QueryPlannerTests::testIntUnaryCondition(
    client::IntCondition_Operation operation, int64_t value,
    int64_t returnedRows, const InternalTable &internalTable) {

  QueryPlan queryPlan;
  queryPlan.schema = internalTable.schema;

  auto intCondtion = client::IntCondition{};
  intCondtion.set_col_name(internalTable.schema->getColumnName(0));
  intCondtion.set_op(operation);
  intCondtion.set_value(value);

  auto unaryCondition = client::UnaryCondition{};
  unaryCondition.mutable_int_condition()->CopyFrom(intCondtion);

  queryPlan.condition.mutable_unary_condition()->CopyFrom(unaryCondition);

  queryPlan.operation = QueryOperation::TableScan;

  QueryPlanner planner{queryPlan, queryExecutor_};

  auto executablePlan = planner.plan(internalTable);
  auto result = QueryPlanner::execute(executablePlan);

  ASSERT_EQ(result.table->num_rows(), returnedRows);
}

} // namespace rk::projects::mydb
