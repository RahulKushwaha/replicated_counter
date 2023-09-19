//
// Created by Aman Mahajan on 9/10/23.
//

#include "applications/mydb/backend/QueryPlanner.h"
#include "TestUtils.h"
#include "applications/mydb/backend/QueryExecutor.h"
#include "applications/mydb/backend/RocksDbFactory.h"
#include "applications/mydb/format/FormatTable.h"
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

TEST_F(QueryPlannerTests, scanTableUsingPrimaryKeyWithBinaryCondition) {
  auto internalTable = test_utils::getInternalTable(10, 5, 0, 1, 1, 1);
  queryExecutor_->insert(internalTable, InsertOptions{InsertOptions::REPLACE});
  auto formattedTable = FormatTable::format(internalTable).str();
  LOG(INFO) << formattedTable;

  auto intCondtion1 = client::IntCondition{};
  intCondtion1.set_col_name(internalTable.schema->getColumnName(0));
  intCondtion1.set_op(client::IntCondition_Operation_GT);
  intCondtion1.set_value(2);

  auto unaryCondition1 = client::UnaryCondition{};
  unaryCondition1.mutable_int_condition()->CopyFrom(intCondtion1);

  auto condition1 = client::Condition{};
  condition1.mutable_unary_condition()->CopyFrom(unaryCondition1);

  auto intCondtion2 = client::IntCondition{};
  intCondtion2.set_col_name(internalTable.schema->getColumnName(1));
  intCondtion2.set_op(client::IntCondition_Operation_LT);
  intCondtion2.set_value(16);

  auto unaryCondition2 = client::UnaryCondition{};
  unaryCondition2.mutable_int_condition()->CopyFrom(intCondtion2);

  auto condition2 = client::Condition{};
  condition2.mutable_unary_condition()->CopyFrom(unaryCondition2);

  auto binaryCondition = client::BinaryCondition{};
  binaryCondition.mutable_c1()->CopyFrom(condition1);
  binaryCondition.mutable_c2()->CopyFrom(condition2);
  binaryCondition.set_op(client::LogicalOperator::AND);

  client::Condition mainCondition;
  mainCondition.mutable_binary_condition()->CopyFrom(binaryCondition);

  QueryPlan queryPlan{
      .operation = QueryOperation::TableScan,
      .schema = internalTable.schema,
      .condition = mainCondition,
  };

  QueryPlanner planner{queryPlan, queryExecutor_};

  auto executablePlan = planner.plan(internalTable);
  auto result = QueryPlanner::execute(executablePlan);

  ASSERT_EQ(result.table->num_rows(), 3);
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
