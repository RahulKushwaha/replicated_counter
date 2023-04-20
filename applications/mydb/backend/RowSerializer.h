//
// Created by Rahul  Kushwaha on 4/7/23.
//
#pragma once

#include "TableRow.h"
#include "AddTableRowRequest.h"
#include "folly/Conv.h"
#include "KeySerializer.h"
#include <arrow/builder.h>

namespace rk::projects::mydb {

constexpr std::string_view EMPTY_ROW_VALUE;

class RowSerializer {
 public:

  static std::vector<RawTableRow> serialize(InternalTable internalTable) {
    std::vector<RawTableRow> rows;

    auto &arrowTable = *internalTable.table;
    for (std::int32_t rowIdx = 0; rowIdx < arrowTable.num_rows();
         rowIdx++) {
      auto totalChunks = arrowTable.columns().front()->num_chunks();
      for (int chunkIdx = 0; chunkIdx < totalChunks; chunkIdx++) {
        std::map < TableSchemaType::ColumnIdType, ColumnValue > colValues;
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

  static std::vector<RawTableRow::Key>
  serializePrimaryKeys(InternalTable internalTable) {
    std::vector<RawTableRow::Key> keys;

    auto &arrowTable = *internalTable.table;
    auto &rawTable = internalTable.schema->rawTable();
    for (std::int32_t rowIdx = 0; rowIdx < arrowTable.num_rows();
         rowIdx++) {
      auto totalChunks = arrowTable.columns().front()->num_chunks();
      for (int chunkIdx = 0; chunkIdx < totalChunks; chunkIdx++) {
        std::vector<ColumnValue> primaryIdxValues;

        for (auto colId: rawTable.primary_key_index().column_ids()) {
          auto col = internalTable.schema->getColumn(colId);
          auto arrowCol =
              arrowTable.GetColumnByName(col.name())->chunk(chunkIdx);
          ColumnValue columnValue;

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

          primaryIdxValues.emplace_back(std::move(columnValue));
        }

        auto primaryKey = prefix::primaryKey(rawTable, primaryIdxValues);
        keys.emplace_back(std::move(primaryKey));
      }
    }

    return keys;
  }

  static InternalTable deserialize(std::shared_ptr<TableSchema> schema,
                                   std::vector<RawTableRow> rows) {
    std::map<TableSchemaType::ColumnIdType,
             std::shared_ptr<arrow::ArrayBuilder>> colBuilder;
    std::vector<std::shared_ptr<arrow::Field>> fields;
    for (const auto &col: schema->rawTable().columns()) {
      std::shared_ptr<arrow::DataType> fieldType;
      if (col.column_type() == internal::Column_COLUMN_TYPE_INT64) {
        fieldType = arrow::int64();
        colBuilder[col.id()] = std::make_shared<arrow::Int64Builder>();
      } else {
        fieldType = arrow::utf8();
        colBuilder[col.id()] = std::make_shared<arrow::StringBuilder>();
      }

      fields.emplace_back(arrow::field(col.name(), fieldType));
    }

    auto arrowSchema = arrow::schema(std::move(fields));

    for (auto &row: rows) {
      for (auto &[key, value]: row.keyValues) {
        auto keyFragments = prefix::parseKey(schema->rawTable(), key);
        if (keyFragments.colId.has_value()) {
          auto colId =
              folly::to<TableSchemaType::ColumnIdType>(*keyFragments.colId);
          auto builder = colBuilder[colId];

          assert(schema->getColumnType(colId).has_value());
          switch (*schema->getColumnType(colId)) {
            case internal::Column_COLUMN_TYPE_INT64: {
              auto intBuilder =
                  std::static_pointer_cast<arrow::Int64Builder>(builder);
              auto typedValue = folly::to<std::int64_t>(value);

              assert(intBuilder->Append(typedValue).ok());
            }
              break;
            case internal::Column_COLUMN_TYPE_STRING: {
              auto stringBuilder =
                  std::static_pointer_cast<arrow::StringBuilder>(builder);

              assert(stringBuilder->Append(value).ok());
            }
              break;
            default:
              assert(false);
          }
        }
      }
    }

    std::vector<std::shared_ptr<arrow::Array>> arrays;
    for (auto &[type, builder]: colBuilder) {
      auto result = builder->Finish();
      arrays.emplace_back(result.ValueOrDie());
    }

    auto recordBatch =
        arrow::RecordBatch::Make(arrowSchema, arrays.size(), arrays);
    auto table = arrow::Table::FromRecordBatches({recordBatch}).ValueOrDie();

    return {schema, table};
  }

  static InternalTable
  deserializeSecondaryIndexKeys(std::shared_ptr<TableSchema> schema,
                                std::vector<RawTableRow> rows,
                                TableSchemaType::TableIdType indexId) {
    std::map<TableSchemaType::ColumnIdType,
             std::shared_ptr<arrow::ArrayBuilder>> colBuilder;
    std::vector<std::shared_ptr<arrow::Field>> fields;
    for (const auto
          &colId: schema->rawTable().primary_key_index().column_ids()) {
      auto col = schema->getColumn(colId);
      std::shared_ptr<arrow::DataType> fieldType;
      if (col.column_type() == internal::Column_COLUMN_TYPE_INT64) {
        fieldType = arrow::int64();
        colBuilder[col.id()] = std::make_shared<arrow::Int64Builder>();
      } else {
        fieldType = arrow::utf8();
        colBuilder[col.id()] = std::make_shared<arrow::StringBuilder>();
      }

      fields.emplace_back(arrow::field(col.name(), fieldType));
    }

    auto arrowSchema = arrow::schema(std::move(fields));

    for (auto &row: rows) {
      for (auto &[key, _]: row.keyValues) {
        auto keyFragments = prefix::parseKey(schema->rawTable(), key);
        std::int32_t indexValueItr = 0;

        if (keyFragments.primaryIndex.has_value()) {
          auto
              value = keyFragments.primaryIndex.value().values[indexValueItr++];
          LOG(INFO) << value;
          for (auto &[colId, builder]: colBuilder) {
            switch (*schema->getColumnType(colId)) {
              case internal::Column_COLUMN_TYPE_INT64: {
                auto intBuilder =
                    std::static_pointer_cast<arrow::Int64Builder>(builder);
                auto typedValue = folly::to<std::int64_t>(value);

                assert(intBuilder->Append(typedValue).ok());
              }
                break;
              case internal::Column_COLUMN_TYPE_STRING: {
                auto stringBuilder =
                    std::static_pointer_cast<arrow::StringBuilder>(builder);

                assert(stringBuilder->Append(value).ok());
              }
                break;
              default:
                assert(false);
            }
          }
        }
      }
    }

    std::vector<std::shared_ptr<arrow::Array>> arrays;
    for (auto &[type, builder]: colBuilder) {
      auto result = builder->Finish();
      arrays.emplace_back(result.ValueOrDie());
    }

    auto recordBatch =
        arrow::RecordBatch::Make(arrowSchema, arrays.size(), arrays);
    auto table = arrow::Table::FromRecordBatches({recordBatch}).ValueOrDie();

    return {schema, table};
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