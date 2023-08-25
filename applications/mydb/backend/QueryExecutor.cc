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

    auto rawTableRows = RowSerializer::serialize(internalTable);
    // Delete the row first.
    std::vector<RawTableRow::Key> keysToDelete;
    for (auto &keyValue : rawTableRows) {
      for (auto &[k, v] : keyValue.keyValues) {
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

// Make it asynchronous later

InternalTable QueryExecutor::get(const InternalTable &internalTable) {
  auto keys = RowSerializer::serializePrimaryKeys(internalTable);
  auto kvRows = rocks_->read(keys);
  return RowSerializer::deserialize(internalTable.schema, kvRows);
}

// Pagination is missing
//
InternalTable QueryExecutor::tableScan(InternalTable internalTable,
                                       IndexQueryOptions indexQueryOptions) {
  if (internalTable.schema->rawTable().primary_key_index().id() ==
      indexQueryOptions.indexId) {

    auto prefix = [&internalTable, &indexQueryOptions]() {
      return prefix::minimumIndexKey(
          internalTable.schema->rawTable(),
          internalTable.schema->rawTable().primary_key_index().id());
    }();

    auto seekPosition = [&internalTable, &indexQueryOptions]() {
      if (indexQueryOptions.primaryKeyValues.empty()) {
        return prefix::minimumIndexKey(
            internalTable.schema->rawTable(),
            internalTable.schema->rawTable().primary_key_index().id());
      }

      // total number of values present in the startKey should be equal to
      // the number of columns in primary prefix.
      assert(indexQueryOptions.primaryKeyValues.size() ==
             internalTable.schema->rawTable()
                 .primary_key_index()
                 .column_ids()
                 .size());

      return prefix::maximumIndexKey(internalTable.schema->rawTable(),
                                     indexQueryOptions.indexId,
                                     indexQueryOptions.primaryKeyValues);
    }();

    auto kvRows =
        rocks_->scan({.direction = indexQueryOptions.direction,
                      .maxRowsReturnSize = indexQueryOptions.maxRowsReturnSize,
                      .prefix = prefix,
                      .seekPosition = seekPosition});

    return RowSerializer::deserialize(internalTable.schema, kvRows);
  } else {
    auto prefix = prefix::minimumIndexKey(internalTable.schema->rawTable(),
                                          indexQueryOptions.indexId);

    auto seekPosition = [&internalTable, &indexQueryOptions]() {
      if (indexQueryOptions.primaryKeyValues.empty()) {
        return prefix::minimumIndexKey(internalTable.schema->rawTable(),
                                       indexQueryOptions.indexId);
      }

      // total number of values present in the startKey should be equal to
      // the number of columns in primary prefix.
      assert(indexQueryOptions.primaryKeyValues.size() ==
              internalTable.schema->rawTable()
                  .primary_key_index()
                  .column_ids()
                  .size());

      return prefix::maximumSecondaryIndexKey(
          internalTable.schema->rawTable(), indexQueryOptions.indexId,
          indexQueryOptions.primaryKeyValues,
          indexQueryOptions.secondaryKeyValues);
    }();

    auto kvRows = rocks_->scan(
        {.direction = indexQueryOptions.direction,
         .prefix = prefix,
         .seekPosition = seekPosition,
         .maxRowsReturnSize = indexQueryOptions.maxRowsReturnSize});

    auto primaryKeys = RowSerializer::deserializeSecondaryIndexKeys(
        internalTable.schema, kvRows, indexQueryOptions.indexId);

    return get(primaryKeys);
  }
};

} // namespace rk::projects::mydb