//
// Created by Aman Mahajan on 9/10/23.
//

#include "TestUtils.h"
#include "applications/mydb/backend/QueryExecutor.h"
#include "applications/mydb/backend/QueryPlanner.h"
#include "applications/mydb/format/FormatTable.h"
#include "persistence/tests/RocksTestFixture.h"

#include <gtest/gtest.h>

namespace rk::projects::mydb {

class QueryPlannerTests : public persistence::RocksTestFixture {
 protected:
  QueryPlannerTests()
      : queryExecutor_{std::make_shared<QueryExecutor>(
            std::make_unique<RocksReaderWriter>(db_))} {}

  void testIntUnaryCondition(IntCondition_Operation operation, int64_t value,
                             int64_t returnedRows,
                             const InternalTable& internalTable);

 protected:
  std::shared_ptr<QueryExecutor> queryExecutor_;
};

TEST_F(QueryPlannerTests, scanTableUsingPrimaryIndexWithIntUnaryCondition) {
  auto internalTable = test_utils::getInternalTable(10, 5, 5, 1, 1, 1);
  queryExecutor_->insert(internalTable);
  testIntUnaryCondition(IntCondition_Operation_GT, 5, 4, internalTable);
  testIntUnaryCondition(IntCondition_Operation_GEQ, 5, 5, internalTable);
  testIntUnaryCondition(IntCondition_Operation_LT, 6, 6, internalTable);
  testIntUnaryCondition(IntCondition_Operation_LEQ, 6, 7, internalTable);
  testIntUnaryCondition(IntCondition_Operation_EQ, 5, 1, internalTable);
}

TEST_F(QueryPlannerTests, scanTableUsingPrimaryKeyWithBinaryCondition) {
  auto internalTable = test_utils::getInternalTable(10, 5, 5, 1, 1, 1);
  queryExecutor_->insert(internalTable);
  auto formattedTable = FormatTable::format(internalTable).str();
  LOG(INFO) << formattedTable;

  auto intCondition1 = IntCondition{};
  intCondition1.set_col_name(internalTable.schema->getColumnName(0));
  intCondition1.set_op(IntCondition_Operation_GT);
  intCondition1.set_value(2);

  auto unaryCondition1 = UnaryCondition{};
  unaryCondition1.mutable_int_condition()->CopyFrom(intCondition1);

  auto condition1 = Condition{};
  condition1.mutable_unary_condition()->CopyFrom(unaryCondition1);

  auto intCondition2 = IntCondition{};
  intCondition2.set_col_name(internalTable.schema->getColumnName(1));
  intCondition2.set_op(IntCondition_Operation_LT);
  intCondition2.set_value(16);

  auto unaryCondition2 = UnaryCondition{};
  unaryCondition2.mutable_int_condition()->CopyFrom(intCondition2);

  auto condition2 = Condition{};
  condition2.mutable_unary_condition()->CopyFrom(unaryCondition2);

  auto binaryCondition = BinaryCondition{};
  binaryCondition.mutable_c1()->CopyFrom(condition1);
  binaryCondition.mutable_c2()->CopyFrom(condition2);
  binaryCondition.set_op(LogicalOperator::AND);

  Condition mainCondition;
  mainCondition.mutable_binary_condition()->CopyFrom(binaryCondition);

  QueryPlan queryPlan{
      .operation = QueryOperation::TableScan,
      .schema = internalTable.schema,
      .condition = mainCondition,
  };

  QueryPlanner planner{queryPlan, queryExecutor_};

  auto executablePlan =
      planner.plan(InternalTable{.schema = internalTable.schema});
  auto result = QueryPlanner::execute(executablePlan);

  ASSERT_EQ(result.table->num_rows(), 3);
}

void QueryPlannerTests::testIntUnaryCondition(
    IntCondition_Operation operation, int64_t value, int64_t returnedRows,
    const InternalTable& internalTable) {

  QueryPlan queryPlan;
  queryPlan.schema = internalTable.schema;

  auto intCondtion = IntCondition{};
  intCondtion.set_col_name(internalTable.schema->getColumnName(0));
  intCondtion.set_op(operation);
  intCondtion.set_value(value);

  auto unaryCondition = UnaryCondition{};
  unaryCondition.mutable_int_condition()->CopyFrom(intCondtion);

  queryPlan.condition.mutable_unary_condition()->CopyFrom(unaryCondition);

  queryPlan.operation = QueryOperation::TableScan;

  QueryPlanner planner{queryPlan, queryExecutor_};

  auto executablePlan =
      planner.plan(InternalTable{.schema = internalTable.schema});
  auto result = QueryPlanner::execute(executablePlan);

  ASSERT_EQ(result.table->num_rows(), returnedRows);
}

}  // namespace rk::projects::mydb
