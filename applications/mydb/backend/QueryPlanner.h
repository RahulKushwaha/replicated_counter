//
// Created by Rahul  Kushwaha on 9/6/23.
//

#pragma once
#include "QueryExecutor.h"
#include "applications/mydb/backend/QueryPlan.h"
#include "arrow/acero/exec_plan.h"
#include "arrow/result.h"

namespace rk::projects::mydb {

namespace cp = ::arrow::compute;
namespace ac = ::arrow::acero;

class QueryPlanner {

 public:
  QueryPlanner(QueryPlan plan, std::shared_ptr<QueryExecutor> queryExecutor);

  ExecutableQueryPlan plan(const InternalTable& internalTable);

  static InternalTable execute(const ExecutableQueryPlan& executableQueryPlan);

 private:
  static cp::Expression parseUnaryCondition(
      const InternalTable& internalTable,
      const client::UnaryCondition& unaryCondition);

  cp::Expression parseCondition(const InternalTable& internalTable,
                                const client::Condition& condition);

 private:
  QueryPlan plan_;
  std::shared_ptr<QueryExecutor> queryExecutor_;
};

}  // namespace rk::projects::mydb
