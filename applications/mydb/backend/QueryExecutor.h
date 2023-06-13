//
// Created by Rahul  Kushwaha on 4/11/23.
//

#pragma once

#include "RocksReaderWriter.h"
#include "TableRow.h"

namespace rk::projects::mydb {

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