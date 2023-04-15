//
// Created by Rahul  Kushwaha on 4/11/23.
//

#pragma once

#include "TableRow.h"
#include "RocksReaderWriter.h"

namespace rk::projects::mydb {
enum class InsertOptions {
  REPLACE,
  MERGE
};

class QueryExecutor {
 public:
  explicit QueryExecutor(std::shared_ptr<RocksReaderWriter> rocks);
 public:
  void insert(const InternalTable &internalTable, InsertOptions options);
  InternalTable get(const InternalTable &internalTable);

  InternalTable tableScan(InternalTable internalTable);

 private:


 private:
  std::shared_ptr<RocksReaderWriter> rocks_;

};

}