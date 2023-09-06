//
// Created by Rahul  Kushwaha on 9/6/23.
//

#pragma once
#include "applications/mydb/backend/QueryPlan.h"

namespace rk::projects::mydb {

class QueryPlanner {
public:
  explicit QueryPlanner(QueryPlan plan);

  ExecutableQueryPlan plan();

private:
  QueryPlan plan_;
};

} // namespace rk::projects::mydb
