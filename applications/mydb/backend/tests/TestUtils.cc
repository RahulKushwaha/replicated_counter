//
// Created by Rahul  Kushwaha on 4/7/23.
//

#include "TestUtils.h"

#include "arrow/table.h"

#include <arrow/compute/api.h>

namespace rk::projects::mydb::test_utils {

TableSchemaOutput parse(const std::string& key) {
  std::string str;
  std::stringstream ss(key);

  TableSchemaOutput schema{};

  int index = 0;
  while (std::getline(ss, str, '/') && index < 3) {
    switch (index) {
      case 0:
        schema.dbId = folly::to<uint32_t>(str);
        break;

      case 1:
        schema.tableId = folly::to<uint32_t>(str);
        break;

      case 2:
        schema.indexId = folly::to<uint32_t>(str);
        break;

      default:
        throw std::runtime_error{"unknown case"};
    }

    index++;
  }

  return schema;
}

TableSchema createTableSchema(int numIntColumns, int numStringColumns,
                              int numColumnsInPrimaryIndex,
                              int numSecondaryIndex,
                              int numColumnsInSecondaryIndex) {
  int totalColumns = numIntColumns + numStringColumns;

  assert(numColumnsInPrimaryIndex < totalColumns);
  assert(numColumnsInSecondaryIndex < totalColumns);

  internal::Database database{};
  database.set_name("test");
  database.set_id(1);

  internal::Table table{};
  table.mutable_db()->CopyFrom(database);
  table.set_name("test_table");
  table.set_id(1);

  TableSchemaType::ColumnIdType colId{0};
  TableSchemaType::TableIdType tableId{0};

  internal::PrimaryKeyIndex primaryKeyIndex{};
  primaryKeyIndex.set_id(tableId++);
  for (int i = 0; i < numColumnsInPrimaryIndex; i++) {
    primaryKeyIndex.mutable_column_ids()->Add(i);
  }

  for (int i = 0; i < numIntColumns; i++) {
    Column column{};
    column.set_id(colId++);
    column.set_name("col_int_name_" + std::to_string(colId));
    column.set_column_type(internal::Column_COLUMN_TYPE_INT64);
    table.mutable_columns()->Add(std::move(column));
  }

  for (int i = 0; i < numStringColumns; i++) {
    Column column{};
    column.set_id(colId++);
    column.set_name("col_str_name_" + std::to_string(colId));
    column.set_column_type(internal::Column_COLUMN_TYPE_STRING);
    table.mutable_columns()->Add(std::move(column));
  }

  assert(numSecondaryIndex < colId);
  for (int i = 1; i <= numSecondaryIndex; i++) {
    SecondaryIndex idx;
    idx.set_name("secondary_idx");
    idx.set_id(tableId++);
    int indexColumnId = i;

    for (int j = 0; j < numColumnsInSecondaryIndex; j++) {

      idx.add_column_ids(indexColumnId);
      indexColumnId++;
    }

    table.mutable_secondary_index()->Add(std::move(idx));
  }

  table.mutable_primary_key_index()->CopyFrom(primaryKeyIndex);

  std::shared_ptr<Table> tablePtr = std::make_shared<Table>(table);

  TableSchema tableSchema{tablePtr};

  return tableSchema;
}

InternalTable getInternalTable(std::int32_t numRows, int numIntColumns,
                               int numStringColumns,
                               int numColumnsInPrimaryIndex,
                               int numSecondaryIndex,
                               int numColumnsInSecondaryIndex) {
  auto tableSchema = std::make_shared<TableSchema>(createTableSchema(
      numIntColumns, numStringColumns, numColumnsInPrimaryIndex,
      numSecondaryIndex, numColumnsInSecondaryIndex));
  std::vector<std::shared_ptr<arrow::Array>> columns;
  std::vector<std::shared_ptr<arrow::Field>> fields;
  int key = 0;
  for (auto& col : tableSchema->rawTable().columns()) {
    switch (col.column_type()) {
      case Column_COLUMN_TYPE_INT64: {
        fields.emplace_back(arrow::field(col.name(), arrow::int64()));
        arrow::Int64Builder builder;
        for (int i = 0; i < numRows; i++) {
          builder.Append(key++);
        }

        columns.emplace_back(builder.Finish().ValueOrDie());
      } break;
      case Column_COLUMN_TYPE_STRING: {
        fields.emplace_back(arrow::field(col.name(), arrow::utf8()));
        arrow::StringBuilder builder;
        for (int i = 0; i < numRows; i++) {
          builder.Append(col.name() + "-value-" + std::to_string(i));
        }

        columns.emplace_back(builder.Finish().ValueOrDie());
      } break;
      default:
        throw std::runtime_error{"unknown column type"};
    }
  }

  auto schema = arrow::schema(fields);

  auto recordBatch = arrow::RecordBatch::Make(schema, numRows, columns);
  auto table = arrow::Table::FromRecordBatches({recordBatch}).ValueOrDie();

  return {tableSchema, table};
}

std::vector<ColumnValue> parsePrimaryKeyValues(InternalTable internalTable) {
  auto& arrowTable = *internalTable.table;
  auto& rawTable = internalTable.schema->rawTable();

  for (std::int32_t rowIdx = 0; rowIdx < arrowTable.num_rows(); rowIdx++) {
    auto totalChunks = arrowTable.columns().front()->num_chunks();
    for (int chunkIdx = 0; chunkIdx < totalChunks; chunkIdx++) {
      std::vector<ColumnValue> primaryIdxValues;
      LOG(INFO) << "ToTAL COlS: "
                << rawTable.primary_key_index().column_ids().size();
      for (auto colId : rawTable.primary_key_index().column_ids()) {
        auto col = internalTable.schema->getColumn(colId);
        auto arrowCol = arrowTable.GetColumnByName(col.name())->chunk(chunkIdx);
        ColumnValue columnValue;

        if (col.column_type() ==
            mydb::Column_COLUMN_TYPE::Column_COLUMN_TYPE_INT64) {
          auto chunk = std::static_pointer_cast<arrow::Int64Array>(arrowCol);
          auto val = chunk->Value(rowIdx);
          LOG(INFO) << val;
          columnValue = {val};
        } else if (col.column_type() ==
                   mydb::Column_COLUMN_TYPE::Column_COLUMN_TYPE_STRING) {
          auto chunk = std::static_pointer_cast<arrow::StringArray>(arrowCol);
          auto val = chunk->Value(rowIdx);
          LOG(INFO) << val;
          columnValue = {std::string{val}};
        }

        primaryIdxValues.emplace_back(std::move(columnValue));
      }

      return primaryIdxValues;
    }
  }

  return {};
}

std::vector<ColumnValue> parseSecondaryKeyValues(InternalTable internalTable,
                                                 int secondarykeyIndex) {
  auto& arrowTable = *internalTable.table;
  auto& rawTable = internalTable.schema->rawTable();

  for (std::int32_t rowIdx = 0; rowIdx < arrowTable.num_rows(); rowIdx++) {
    auto totalChunks = arrowTable.columns().front()->num_chunks();
    for (int chunkIdx = 0; chunkIdx < totalChunks; chunkIdx++) {
      std::vector<ColumnValue> secondaryIdxValues;
      LOG(INFO) << "ToTAL COlS: "
                << rawTable.primary_key_index().column_ids().size();

      for (auto colId : rawTable.secondary_index(0).column_ids()) {
        auto col = internalTable.schema->getColumn(colId);
        auto arrowCol = arrowTable.GetColumnByName(col.name())->chunk(chunkIdx);
        ColumnValue columnValue;

        if (col.column_type() ==
            mydb::Column_COLUMN_TYPE::Column_COLUMN_TYPE_INT64) {
          auto chunk = std::static_pointer_cast<arrow::Int64Array>(arrowCol);
          auto val = chunk->Value(rowIdx);
          LOG(INFO) << val;
          columnValue = {val};
        } else if (col.column_type() ==
                   mydb::Column_COLUMN_TYPE::Column_COLUMN_TYPE_STRING) {
          auto chunk = std::static_pointer_cast<arrow::StringArray>(arrowCol);
          auto val = chunk->Value(rowIdx);
          LOG(INFO) << val;
          columnValue = {std::string{val}};
        }

        secondaryIdxValues.emplace_back(std::move(columnValue));
      }

      return secondaryIdxValues;
    }
  }

  return {};
}

}  // namespace rk::projects::mydb::test_utils