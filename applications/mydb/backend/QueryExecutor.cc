//
// Created by Rahul  Kushwaha on 4/11/23.
//

#include "QueryExecutor.h"
#include "RowSerializer.h"

namespace rk::projects::mydb {

QueryExecutor::QueryExecutor(std::shared_ptr<RocksReaderWriter> rocks)
    : rocks_{std::move(rocks)} {}

void QueryExecutor::insert(const InternalTable &internalTable) {
  auto kvRows = RowSerializer::serialize(internalTable);
  rocks_->write(kvRows);
}

InternalTable QueryExecutor::get(const InternalTable &internalTable) {
  auto keys = RowSerializer::serializePrimaryKeys(internalTable);
  auto kvRows = rocks_->read(keys);
  return RowSerializer::deserialize(internalTable.schema, kvRows);
}

}