//
// Created by Rahul  Kushwaha on 4/11/23.
//

#include "QueryExecutor.h"
#include "RowSerializer.h"

namespace rk::projects::mydb {

QueryExecutor::QueryExecutor(std::shared_ptr<RocksReaderWriter> rocks)
    : rocks_{std::move(rocks)} {}

void QueryExecutor::insert(const InternalTable &internalTable,
                           InsertOptions option) {
  if (option == InsertOptions::REPLACE) {
    // Delete the row first.
    auto rawTableRows = RowSerializer::serialize(internalTable);
    std::vector<RawTableRow::Key> keysToDelete;
    for (auto &keyValue: rawTableRows) {
      for (auto &[k, v]: keyValue.keyValues) {
        keysToDelete.emplace_back(k);
      }
    }

    rocks_->del(keysToDelete);
    // Insert the row.
    rocks_->write(rawTableRows);
    return;
  }

  assert(option == InsertOptions::MERGE);
}

InternalTable QueryExecutor::get(const InternalTable &internalTable) {
  auto keys = RowSerializer::serializePrimaryKeys(internalTable);
  auto kvRows = rocks_->read(keys);
  return RowSerializer::deserialize(internalTable.schema, kvRows);
}

InternalTable QueryExecutor::tableScan(InternalTable internalTable,
                                       IndexQueryOptions indexQueryOptions) {
  if (internalTable.schema->rawTable().primary_key_index().id()
      == indexQueryOptions.indexId) {

    auto key = [&internalTable, &indexQueryOptions]() {
      if (indexQueryOptions.startFromKey.empty()) {
        return prefix::minimumIndexKey(internalTable.schema->rawTable(),
                                       internalTable.schema->rawTable().primary_key_index().id());
      }

      // total number of values present in the startKey should be equal to
      // the number of columns in primary key.
      assert(indexQueryOptions.startFromKey.size()
                 == internalTable.schema->rawTable().primary_key_index().column_ids().size());

      return prefix::maximumIndexKey(internalTable.schema->rawTable(),
                                     indexQueryOptions.indexId,
                                     indexQueryOptions.startFromKey);
    }();

    LOG(INFO) << "Scan Table Key: " << key;
    auto kvRows = rocks_->scan(key, {
        .direction = indexQueryOptions.direction,
        .maxRowsReturnSize = indexQueryOptions.maxRowsReturnSize
    });

    return RowSerializer::deserialize(internalTable.schema, kvRows);
  } else {
    auto key = prefix::minimumIndexKey(internalTable.schema->rawTable(),
                                       indexQueryOptions.indexId);

    auto kvRows = rocks_->scan(key, {.direction = indexQueryOptions.direction});

    auto primaryKeys = RowSerializer::deserializeSecondaryIndexKeys(
        internalTable.schema,
        kvRows, indexQueryOptions.indexId);

    return primaryKeys;
  }
};

}