//
// Created by Rahul  Kushwaha on 9/6/23.
//

#include "applications/mydb/backend/QueryPlanner.h"

#include "applications/mydb/format/FormatTable.h"
#include "fmt/chrono.h"

#include <utility>

namespace rk::projects::mydb {
QueryPlanner::QueryPlanner(QueryPlan plan,
                           std::shared_ptr<QueryExecutor> queryExecutor)
    : plan_(std::move(plan)), queryExecutor_(std::move(queryExecutor)) {}

ExecutableQueryPlan QueryPlanner::plan(const InternalTable& internalTable) {
  IndexQueryOptions indexQueryOptions{
      .indexId = internalTable.schema->rawTable().primary_key_index().id(),
      .direction = ScanDirection::FORWARD,
      .maxRowsReturnSize = 100000};

  auto table = queryExecutor_->tableScan(internalTable, indexQueryOptions);
  auto tableSourceOptions = ac::TableSourceNodeOptions{table.table};

  ac::Declaration source{"table_source", std::move(tableSourceOptions)};

  if (plan_.condition.has_binary_condition() ||
      plan_.condition.has_unary_condition()) {
    auto filterExpr = parseCondition(internalTable, plan_.condition);

    ac::Declaration filter{
        "filter", {source}, ac::FilterNodeOptions(std::move(filterExpr))};
    source = filter;
  }

  return ExecutableQueryPlan{
      .plan = source,
      .inputPlan = plan_,
  };
}

// TODO : Make it iterative.
cp::Expression QueryPlanner::parseCondition(
    const InternalTable& internalTable, const client::Condition& condition) {
  cp::Expression filterExpression;
  if (condition.has_unary_condition()) {
    return parseUnaryCondition(internalTable, condition.unary_condition());
  }

  if (condition.has_binary_condition()) {
    auto left =
        parseCondition(internalTable, condition.binary_condition().c1());
    auto right =
        parseCondition(internalTable, condition.binary_condition().c2());

    switch (condition.binary_condition().op()) {
      case client::AND:
        filterExpression = cp::and_(left, right);
        break;
      case client::OR:
        filterExpression = cp::or_(left, right);
        break;
      default:
        assert(false && "unknown logical joining condition");
    }
    return filterExpression;
  }

  return cp::literal(true);
}

cp::Expression QueryPlanner::parseUnaryCondition(
    const InternalTable& internalTable,
    const client::UnaryCondition& unaryCondition) {

  if (unaryCondition.has_int_condition()) {
    std::string colName = unaryCondition.int_condition().col_name();
    auto value = unaryCondition.int_condition().value();
    cp::Expression filterExpr;

    switch (unaryCondition.int_condition().op()) {
      case client::IntCondition_Operation_EQ:
        filterExpr = cp::equal(cp::field_ref(colName), cp::literal(value));
        break;
      case client::IntCondition_Operation_LEQ:

        filterExpr = cp::less_equal(cp::field_ref(colName), cp::literal(value));
        break;
      case client::IntCondition_Operation_LT:
        filterExpr = cp::less(cp::field_ref(colName), cp::literal(value));
        break;
      case client::IntCondition_Operation_GEQ:
        filterExpr =
            cp::greater_equal(cp::field_ref(colName), cp::literal(value));
        break;
      case client::IntCondition_Operation_GT:
        filterExpr = cp::greater(cp::field_ref(colName), cp::literal(value));
        break;
      default:
        assert(false);
    }

    return filterExpr;
  }

  if (unaryCondition.has_string_condition()) {

    std::string colName = unaryCondition.string_condition().name();
    auto value = unaryCondition.string_condition().value();
    cp::Expression filter_expr;

    // TODO : figure it out the string part.
    switch (unaryCondition.string_condition().op()) {

      case client::StringCondition_Operation_EQ:
        filter_expr = cp::equal(cp::field_ref(colName), cp::literal(value));
        break;
      case client::StringCondition_Operation_HAS_SUBSTR:

        break;
      case client::StringCondition_Operation_STARTS_WITH:
        break;
      case client::StringCondition_Operation_ENDS_WITH:
        break;
      case client::StringCondition_Operation_CASE_INSENSITIVE_EQ:
        break;
      case client::StringCondition_Operation_CASE_INSENSITIVE_HAS_SUBSTR:
        break;
      case client::StringCondition_Operation_CASE_INSENSITIVE_STARTS_WITH:
        break;
      case client::StringCondition_Operation_CASE_INSENSITIVE_ENDS_WITH:
        break;

      default:
        assert(false);
    }

    return filter_expr;
  }

  assert(false);
}

/**
 * TODO : Remove this method.
 *
 * */

InternalTable QueryPlanner::execute(
    const ExecutableQueryPlan& executableQueryPlan) {

  auto status = ac::DeclarationToTable(executableQueryPlan.plan);
  if (!status.ok()) {
    LOG(ERROR) << status.status().ToString();
    assert(status.ok());
  }

  std::shared_ptr<arrow::Table> responseTable = status.ValueOrDie();
  return InternalTable{.schema = executableQueryPlan.inputPlan.schema,
                       .table = responseTable};
}

}  // namespace rk::projects::mydb
