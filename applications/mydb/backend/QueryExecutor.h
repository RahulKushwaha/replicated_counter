//
// Created by Rahul  Kushwaha on 4/11/23.
//

#pragma once

#include "RocksReaderWriter.h"
#include "TableRow.h"
#include "arrow/acero/options.h"
#include "folly/experimental/coro/AsyncGenerator.h"

namespace rk::projects::mydb {

namespace cp = ::arrow::compute;
namespace ac = ::arrow::acero;

class QueryExecutor {
 public:
  explicit QueryExecutor(std::shared_ptr<RocksReaderWriter> rocks);

 public:
  void insert(const InternalTable& internalTable);

  void update(const InternalTable& internalTable, UpdateOptions option);

  InternalTable get(const InternalTable& internalTable);

  InternalTable tableScan(InternalTable internalTable,
                          IndexQueryOptions queryOptions);

  folly::coro::AsyncGenerator<InternalTable> tableScanGenerator(
      InternalTable internalTable, IndexQueryOptions queryOptions);

 private:
  std::shared_ptr<RocksReaderWriter> rocks_;
};

}  // namespace rk::projects::mydb