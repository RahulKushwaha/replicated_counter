//
// Created by Rahul  Kushwaha on 4/11/23.
//

#pragma once

#include "QueryPlan.h"
#include "RocksReaderWriter.h"
#include "TableRow.h"
#include "arrow/acero/options.h"
#include "arrow/result.h"

namespace rk::projects::mydb {

namespace cp = ::arrow::compute;
namespace ac = ::arrow::acero;

class QueryExecutor {
public:
  explicit QueryExecutor(std::shared_ptr<RocksReaderWriter> rocks);

public:
  void insert(const InternalTable &internalTable, InsertOptions option);

  InternalTable get(const InternalTable &internalTable);

  InternalTable tableScan(InternalTable internalTable,
                          IndexQueryOptions queryOptions);

private:
private:
  std::shared_ptr<RocksReaderWriter> rocks_;
};

} // namespace rk::projects::mydb