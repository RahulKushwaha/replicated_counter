//
// Created by Rahul  Kushwaha on 4/7/23.
//
#pragma once

#include "TableRow.h"
#include "AddTableRowRequest.h"
#include "folly/Conv.h"
#include "KeySerializer.h"

namespace rk::projects::mydb {

constexpr std::string_view EMPTY_ROW_VALUE;

class RowSerializer {
 public:

  static std::vector<RawTableRow> serialize(InternalTable internalTable) {
    std::vector<RawTableRow> rows;

    auto &arrowTable = *internalTable.table;
    for (std::int32_t rowIdx = 0; rowIdx < arrowTable.num_rows();
         rowIdx++) {
      LOG(INFO) << "iteration: " << rowIdx;
      auto totalChunks = arrowTable.columns().front()->num_chunks();
      for (int chunkIdx = 0; chunkIdx < totalChunks; chunkIdx++) {
        LOG(INFO) << "chunk: " << chunkIdx;
        std::map<TableSchemaType::ColumnIdType, ColumnValue> colValues;
        std::vector<ColumnValue> primaryIdxValues;
        std::unordered_map<TableSchemaType::TableIdType,
                           std::vector<ColumnValue >> secondaryIdxValues;

        for (const auto &col: internalTable.schema->rawTable().columns()) {
          ColumnValue columnValue;
          auto arrowCol =
              arrowTable.GetColumnByName(col.name())->chunk(chunkIdx);

          if (col.column_type()
              == mydb::Column_COLUMN_TYPE::Column_COLUMN_TYPE_INT64) {
            auto chunk = std::static_pointer_cast<arrow::Int64Array>(arrowCol);
            auto val = chunk->Value(rowIdx);
            columnValue = {val};
          } else if (col.column_type()
              == mydb::Column_COLUMN_TYPE::Column_COLUMN_TYPE_STRING) {
            auto chunk = std::static_pointer_cast<arrow::StringArray>(arrowCol);
            auto val = chunk->Value(rowIdx);
            columnValue = {std::string{val}};
          }

          colValues.emplace(col.id(), columnValue);

          if (internalTable.schema->isPrimaryKeyColumn(col.id())) {
            primaryIdxValues.emplace_back(columnValue);
          }

          for (auto idxId: internalTable.schema->indexIds(col.id())) {
            secondaryIdxValues[idxId].push_back(columnValue);
          }
        }

        RawTableRow rawTableRow{};
        auto primaryKey = prefix::primaryKey(internalTable.schema->rawTable(),
                                             primaryIdxValues);

        rawTableRow.keyValues.emplace_back(primaryKey, "NULL");

        for (auto &[colId, colValue]: colValues) {
          std::string colKey = prefix::columnKey(primaryKey, colId);
          rawTableRow.keyValues.emplace_back(std::move(colKey),
                                             toString(colValue));
        }

        for (auto &[idxId, values]: secondaryIdxValues) {
          std::string idxKey =
              prefix::secondaryIndexKey(
                  internalTable.schema->rawTable(), idxId, values);

          rawTableRow.keyValues.emplace_back(std::move(idxKey), "NULL");
        }

        rows.emplace_back(std::move(rawTableRow));
      }
    }

    return rows;
  }

  static ColumnValue toColumnValue(const google::protobuf::Any &columnValue) {
    if (columnValue.type_url() == "std::string") {
      return columnValue.value();
    } else if (columnValue.GetTypeName() == "std::uint64") {
      return folly::to<std::int64_t>(columnValue.value());
    } else {
      LOG(INFO) << "unknown type " << columnValue.type_url();
      throw std::runtime_error{"unknown type for serialization"};
    }
  }
};

}