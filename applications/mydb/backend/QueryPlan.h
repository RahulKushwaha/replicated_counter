//
// Created by Rahul  Kushwaha on 9/6/23.
//

#pragma once
#include "applications/mydb/backend/Common.h"
#include "applications/mydb/client/proto/condition.pb.h"
#include <arrow/acero/exec_plan.h>

namespace rk::projects::mydb {

enum class QueryOperation {
  TableScan,
};

struct QueryPlan {
  QueryOperation operation;
  std::shared_ptr<TableSchema> schema;
  std::vector<TableSchemaType::ColumnIdType> outputColumns;
  client::Condition condition;
};

struct ExecutableQueryPlan {
  arrow::acero::Declaration plan;
  QueryPlan inputPlan;
};

} // namespace rk::projects::mydb