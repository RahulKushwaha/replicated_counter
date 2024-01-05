//
// Created by Rahul  Kushwaha on 4/4/23.
//

#pragma once

#include "applications/mydb/backend/Common.h"
#include "applications/mydb/backend/RowSerializer.h"
#include "applications/mydb/backend/TableRow.h"
#include "applications/mydb/client/proto/db.pb.h"

namespace rk::projects::mydb::internal {

class Transformer {
 public:
  static InternalTable from(std::vector<client::ColumnValue> columnValues,
                            std::shared_ptr<TableSchema> schema) {
    auto [colBuilder, fields] = RowSerializer::getHeader(schema);

    for (auto& columnValue : columnValues) {
      auto colId = schema->getColumnId(columnValue.name().name());
      auto builder = colBuilder.at(colId);

      switch (*schema->getColumnType(colId)) {
        case Column_COLUMN_TYPE_INT64: {
          auto intBuilder =
              std::static_pointer_cast<arrow::Int64Builder>(builder);

          auto typedValue =
              folly::to<std::int64_t>(columnValue.value().value());

          assert(intBuilder->Append(typedValue).ok());
        } break;
        case Column_COLUMN_TYPE_STRING: {
          auto stringBuilder =
              std::static_pointer_cast<arrow::StringBuilder>(builder);

          assert(stringBuilder->Append(columnValue.value().value()).ok());
        } break;
        default:
          assert(false);
      }
    }

    auto arrowSchema = arrow::schema(std::move(fields));

    std::vector<std::shared_ptr<arrow::Array>> arrays;
    for (auto& [type, builder] : colBuilder) {
      auto result = builder->Finish();
      arrays.emplace_back(result.ValueOrDie());
    }

    auto recordBatch =
        arrow::RecordBatch::Make(arrowSchema, arrays[0]->length(), arrays);
    auto table = arrow::Table::FromRecordBatches({recordBatch}).ValueOrDie();

    return {schema, table};
  }

  static client::TableRows from(InternalTable internalTable) {
    client::TableRows tableRows;

    auto& arrowTable = *internalTable.table;
    for (std::int32_t rowIdx = 0; rowIdx < arrowTable.num_rows(); rowIdx++) {
      auto totalChunks = arrowTable.columns().front()->num_chunks();
      for (std::int32_t chunkIdx = 0; chunkIdx < totalChunks; chunkIdx++) {

        client::Row row;
        for (const auto& col : internalTable.schema->rawTable().columns()) {
          auto arrowCol =
              arrowTable.GetColumnByName(col.name())->chunk(chunkIdx);

          if (col.column_type() ==
              mydb::Column_COLUMN_TYPE::Column_COLUMN_TYPE_INT64) {
            auto chunk = std::static_pointer_cast<arrow::Int64Array>(arrowCol);
            auto val = chunk->Value(rowIdx);
            google::protobuf::Any any;
            any.set_value(std::to_string(val));
            row.mutable_column_values()->Add(std::move(any));
          } else if (col.column_type() ==
                     mydb::Column_COLUMN_TYPE::Column_COLUMN_TYPE_STRING) {
            auto chunk = std::static_pointer_cast<arrow::StringArray>(arrowCol);
            auto val = chunk->Value(rowIdx);
            google::protobuf::Any any;
            any.set_value(val);
            row.mutable_column_values()->Add(std::move(any));
          }
        }

        tableRows.mutable_rows()->Add(std::move(row));
      }
    }

    return tableRows;
  }

  static InternalTable from(
      const client::AddDatabaseRequest& addDatabaseRequest,
      std::shared_ptr<TableSchema> schema, TableSchemaType::DbIdType dbId) {
    const auto epoch = std::chrono::system_clock::now().time_since_epoch();
    const auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(epoch).count();

    std::vector<client::ColumnValue> columnValues;
    {
      google::protobuf::Any id;
      id.set_value(std::to_string(dbId));
      client::ColumnValue idValue;
      idValue.mutable_name()->set_name("id");
      *idValue.mutable_value() = std::move(id);

      columnValues.emplace_back(std::move(idValue));
    }

    {
      google::protobuf::Any name;
      name.set_value(addDatabaseRequest.database().name().name());
      client::ColumnValue nameValue;
      nameValue.mutable_name()->set_name("name");
      *nameValue.mutable_value() = std::move(name);

      columnValues.emplace_back(std::move(nameValue));
    }

    {
      google::protobuf::Any created;
      created.set_value(std::to_string(seconds));
      client::ColumnValue createdValue;
      createdValue.mutable_name()->set_name("created");
      *(createdValue.mutable_value()) = std::move(created);

      columnValues.emplace_back(std::move(createdValue));
    }

    {
      google::protobuf::Any modified;
      modified.set_value(std::to_string(seconds));
      client::ColumnValue modifiedValue;
      modifiedValue.mutable_name()->set_name("modified");
      *modifiedValue.mutable_value() = std::move(modified);

      columnValues.emplace_back(std::move(modifiedValue));
    }

    return from(std::move(columnValues), std::move(schema));
  }

  static InternalTable from(const client::AddTableRequest& addTableRequest,
                            std::shared_ptr<TableSchema> schema,
                            TableSchemaType::TableIdType tableId) {
    const auto epoch = std::chrono::system_clock::now().time_since_epoch();
    const auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(epoch).count();

    std::vector<client::ColumnValue> columnValues;
    {
      google::protobuf::Any id;
      id.set_value(std::to_string(tableId));
      client::ColumnValue idValue;
      idValue.mutable_name()->set_name("id");
      *idValue.mutable_value() = std::move(id);

      columnValues.emplace_back(std::move(idValue));
    }

    {
      google::protobuf::Any name;
      name.set_value(addTableRequest.table().name().name());
      client::ColumnValue nameValue;
      nameValue.mutable_name()->set_name("name");
      *nameValue.mutable_value() = std::move(name);

      columnValues.emplace_back(std::move(nameValue));
    }

    {
      google::protobuf::Any created;
      created.set_value(std::to_string(seconds));
      client::ColumnValue createdValue;
      createdValue.mutable_name()->set_name("created");
      *(createdValue.mutable_value()) = std::move(created);

      columnValues.emplace_back(std::move(createdValue));
    }

    {
      google::protobuf::Any modified;
      modified.set_value(std::to_string(seconds));
      client::ColumnValue modifiedValue;
      modifiedValue.mutable_name()->set_name("modified");
      *modifiedValue.mutable_value() = std::move(modified);

      columnValues.emplace_back(std::move(modifiedValue));
    }

    {
      google::protobuf::Any serializedTableString;
      serializedTableString.set_value(addTableRequest.SerializeAsString());
      client::ColumnValue serializedTableStringValue;
      serializedTableStringValue.mutable_name()->set_name(
          "serialized_table_str");
      *serializedTableStringValue.mutable_value() =
          std::move(serializedTableString);

      columnValues.emplace_back(std::move(serializedTableStringValue));
    }

    {
      google::protobuf::Any serializedMaxIdNum;
      serializedMaxIdNum.set_value("0");
      client::ColumnValue serializedMaxIdNumValue;
      serializedMaxIdNumValue.mutable_name()->set_name("max_id_num");
      *serializedMaxIdNumValue.mutable_value() = std::move(serializedMaxIdNum);

      columnValues.emplace_back(std::move(serializedMaxIdNumValue));
    }

    return from(std::move(columnValues), std::move(schema));
  }

  static InternalTable from(const client::AddRowRequest& addRowRequest,
                            std::shared_ptr<TableSchema> schema) {
    std::vector<client::ColumnValue> columnValues{
        addRowRequest.column_values().begin(),
        addRowRequest.column_values().end()};
    return from(columnValues, std::move(schema));
  }

  static InternalTable from(const client::UpdateRowRequest& updateRowRequest,
                            std::shared_ptr<TableSchema> schema) {
    std::vector<client::ColumnValue> columnValues{
        updateRowRequest.column_values().begin(),
        updateRowRequest.column_values().end()};
    return from(columnValues, std::move(schema));
  }

  static internal::Table from(const client::Table& clientTable,
                              std::uint32_t dbId, std::uint32_t tableId) {
    internal::Table table{};
    table.set_id(tableId);
    table.set_name(clientTable.name().name());

    table.mutable_db()->set_name(clientTable.db().name());
    table.mutable_db()->set_id(dbId);

    std::uint32_t idCounter{0};
    std::map<std::string, TableSchemaType::ColumnIdType> idLookup;
    for (auto& clientColumn : clientTable.columns()) {
      internal::Column column{};
      column.set_name(clientColumn.name().name());
      column.set_id(idCounter++);

      idLookup[clientColumn.name().name()] = column.id();

      switch (clientColumn.column_type()) {
        case client::Column_COLUMN_TYPE_INT64:
          column.set_column_type(internal::Column_COLUMN_TYPE_INT64);
          break;
        case client::Column_COLUMN_TYPE_STRING:
          column.set_column_type(internal::Column_COLUMN_TYPE_STRING);
          break;
        default:
          assert(false && "invalid column type");
      }

      table.mutable_columns()->Add(std::move(column));
    }

    auto& primaryKeyIndex = *table.mutable_primary_key_index();
    primaryKeyIndex.set_id(0);
    primaryKeyIndex.set_name("primary_key_idx");
    for (auto& primaryIndexKeyName : clientTable.primary_key().columns()) {
      const auto colId = idLookup.at(primaryKeyIndex.name());
      primaryKeyIndex.add_column_ids(colId);
    }

    for (auto& secondaryIndex : clientTable.secondary_index()) {
      internal::SecondaryIndex idx{};
      idx.set_name(secondaryIndex.name());

      for (auto& col : secondaryIndex.columns()) {
        const auto colId = idLookup.at(col.name());
        idx.add_column_ids(colId);
      }

      table.mutable_secondary_index()->Add(std::move(idx));
    }

    return table;
  }
};

}  // namespace rk::projects::mydb::internal