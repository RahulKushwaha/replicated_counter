//
// Created by Rahul  Kushwaha on 4/11/23.
//

#pragma once

#include "TableRow.h"
#include "RocksReaderWriter.h"

namespace rk::projects::mydb {
class QueryExecutor {
 public:
  explicit QueryExecutor(std::shared_ptr<RocksReaderWriter> rocks);
 public:
  void insert(const InternalTable &internalTable);
  InternalTable get(const InternalTable &internalTable);

 private:
  std::shared_ptr<RocksReaderWriter> rocks_;

};

}