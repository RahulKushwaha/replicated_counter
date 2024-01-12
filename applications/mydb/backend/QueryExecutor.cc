//
// Created by Rahul  Kushwaha on 4/11/23.
//

#include "QueryExecutor.h"

#include "RowSerializer.h"
#include "applications/mydb/format/FormatTable.h"

namespace rk::projects::mydb {

QueryExecutor::QueryExecutor(std::shared_ptr<RocksReaderWriter> rocks)
    : rocks_{std::move(rocks)} {}

void QueryExecutor::insert(const InternalTable& internalTable) {
  auto getResult = get(internalTable);
  if (getResult.table->num_rows() != 0) {
    throw std::runtime_error{"row already exists"};
  }

  auto rawTableRows = RowSerializer::serialize(internalTable);

  // Insert the row.
  rocks_->write(rawTableRows);
}

void QueryExecutor::update(const InternalTable& internalTable,
                           UpdateOptions option) {
  auto rawTableRows = RowSerializer::serialize(internalTable);

  if (option == UpdateOptions::REPLACE) {
    // Delete the row first.
    std::vector<RawTableRow::Key> keysToDelete;
    for (auto& keyValue : rawTableRows) {
      for (auto& [k, v] : keyValue.keyValues) {
        keysToDelete.emplace_back(k);
      }
    }

    rocks_->del(keysToDelete);
  }

  // Insert the row.
  rocks_->write(rawTableRows);
}

// Make it asynchronous later

InternalTable QueryExecutor::get(const InternalTable& internalTable) {
  auto keys = RowSerializer::serializePrimaryKeys(internalTable);
  auto kvRows = rocks_->read(keys);
  return RowSerializer::deserialize(internalTable.schema, kvRows);
}

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
                      .seekPosition = seekPosition,
                      .prefix = prefix});

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

    auto kvRows =
        rocks_->scan({.direction = indexQueryOptions.direction,
                      .maxRowsReturnSize = indexQueryOptions.maxRowsReturnSize,
                      .seekPosition = seekPosition,
                      .prefix = prefix});

    auto primaryKeys = RowSerializer::deserializeSecondaryIndexKeys(
        internalTable.schema, kvRows, indexQueryOptions.indexId);

    return get(primaryKeys);
  }
}

folly::coro::AsyncGenerator<InternalTable> QueryExecutor::tableScanGenerator(
    InternalTable internalTable, IndexQueryOptions indexQueryOptions) {

  while (true) {

    InternalTable response = tableScan(
        InternalTable{.schema = internalTable.schema}, indexQueryOptions);
    if (response.table->num_rows() == 0) {
      break;
    }
    auto lastRow =
        response.table->Slice(indexQueryOptions.maxRowsReturnSize - 1, 1);
    indexQueryOptions.primaryKeyValues =
        RowSerializer::parsePrimaryKeyValues({internalTable.schema, lastRow});

    if (internalTable.schema->rawTable().primary_key_index().id() !=
        indexQueryOptions.indexId) {
      indexQueryOptions.secondaryKeyValues =
          RowSerializer::parseSecondaryKeyValues(internalTable,
                                                 indexQueryOptions.indexId);
    }
    co_yield response;
  }
}
}  // namespace rk::projects::mydb