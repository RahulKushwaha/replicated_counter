//
// Created by Rahul  Kushwaha on 4/11/23.
//

#include "QueryExecutor.h"
#include "RowSerializer.h"

namespace rk::projects::mydb {

QueryExecutor::QueryExecutor(std::shared_ptr<RocksReaderWriter> rocks)
    : rocks_{std::move(rocks)} {}

void QueryExecutor::insert(const InternalTable &internalTable,
                           InsertOptions options) {
  auto kvRows = RowSerializer::serialize(internalTable);
  rocks_->write(kvRows);
}

InternalTable QueryExecutor::get(const InternalTable &internalTable) {
  auto keys = RowSerializer::serializePrimaryKeys(internalTable);
  auto kvRows = rocks_->read(keys);
  return RowSerializer::deserialize(internalTable.schema, kvRows);
}

InternalTable QueryExecutor::tableScan(InternalTable internalTable) {
  auto key = prefix::minimumIndexKey(internalTable.schema->rawTable(),
                                     internalTable.schema->rawTable().primary_key_index().id());

  auto kvRows = rocks_->scan(key, RocksReaderWriter::ScanDirection::Forward);

  return RowSerializer::deserialize(internalTable.schema, kvRows);
}

}