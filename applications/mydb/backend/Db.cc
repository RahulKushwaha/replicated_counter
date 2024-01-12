//
// Created by rahul on 12/25/23.
//

#include "applications/mydb/backend/Db.h"

#include "applications/mydb/backend/Errors.h"
#include "applications/mydb/backend/QueryPlan.h"
#include "applications/mydb/backend/QueryPlanner.h"
#include "applications/mydb/backend/RequestTransformer.h"
#include "applications/mydb/format/FormatTable.h"

namespace rk::projects::mydb {

Db::Db(std::shared_ptr<SchemaStore> schemaStore,
       std::shared_ptr<RocksReaderWriter> rocks)
    : schemaStore_{std::move(schemaStore)},
      queryExecutor_{std::make_shared<QueryExecutor>(std::move(rocks))},
      dbIds_{0},
      tableIds_{0} {}

google::protobuf::Empty Db::addDatabase(
    const client::AddDatabaseRequest* request) {
  auto table = schemaStore_->getTable("meta", "meta_db");
  assert(table.has_value() && "table not found");

  auto tableSchema =
      std::make_shared<TableSchema>(std::make_shared<Table>(table.value()));
  const auto dbId = dbIds_++;

  auto internalTable = Transformer::from(*request, tableSchema, dbId);

  queryExecutor_->insert(internalTable);

  internal::Database database{};
  database.set_id(dbId);
  database.set_name(request->database().name().name());

  const auto result =
      schemaStore_->registerDb(request->database().name().name(), database);
  if (!result) {
    throw DbError{ErrorCode::DATABASE_ALREADY_EXISTS};
  }

  return google::protobuf::Empty{};
}

google::protobuf::Empty Db::addTable(const client::AddTableRequest* request) {
  const auto db = request->table().db();
  auto metaDbTable = schemaStore_->getTable("meta", "meta_db_table");
  assert(metaDbTable.has_value() && "table not found");

  // make sure the database in which the table is being inserted exists
  auto database = schemaStore_->getDatabase(db.name());

  if (!database.has_value()) {
    throw DbError{ErrorCode::DATABASE_DOES_NOT_EXIST};
  }

  auto table =
      schemaStore_->getTable(db.name(), request->table().name().name());

  if (table.has_value()) {
    throw DbError{ErrorCode::TABLE_ALREADY_EXISTS};
  }

  const auto tableId = tableIds_++;

  auto tableSchema = std::make_shared<TableSchema>(
      std::make_shared<Table>(metaDbTable.value()));
  auto internalTable = Transformer::from(*request, tableSchema, tableId);

  queryExecutor_->insert(internalTable);

  auto result = schemaStore_->registerTable(
      db.name(), request->table().name().name(),
      Transformer::from(request->table(), database.value().id(), tableId));

  assert(result && "table already exists");

  return google::protobuf::Empty{};
}

client::AddRowResponse Db::addRow(const client::AddRowRequest* request) {
  auto table = schemaStore_->getTable(request->database_name().name(),
                                      request->table_name().name());
  assert(table.has_value() && "table not found");

  auto tableSchema =
      std::make_shared<TableSchema>(std::make_shared<Table>(table.value()));
  auto internalTable = Transformer::from(*request, tableSchema);

  queryExecutor_->insert(internalTable);

  auto insertedRow = queryExecutor_->get(internalTable);

  client::AddRowResponse response{};
  auto outputRows = Transformer::from(insertedRow);
  *response.mutable_table_rows() = std::move(outputRows);
  return response;
}

client::UpdateRowResponse Db::updateRow(
    const client::UpdateRowRequest* request) {
  auto table = schemaStore_->getTable(request->database_name().name(),
                                      request->table_name().name());
  assert(table.has_value() && "table not found");

  auto tableSchema =
      std::make_shared<TableSchema>(std::make_shared<Table>(table.value()));

  QueryPlan queryPlan{
      .operation = QueryOperation::TableScan,
      .schema = tableSchema,
      .condition = request->condition(),
  };

  QueryPlanner queryPlanner{queryPlan, queryExecutor_};
  auto executableQueryPlan =
      queryPlanner.plan(InternalTable{.schema = tableSchema});
  auto result = QueryPlanner::execute(executableQueryPlan);

  if (result.table->num_rows() == 0) {
    throw DbError{ErrorCode::UPDATE_CONDITION_FAILED};
  }

  auto internalTable = Transformer::from(*request, tableSchema);
  UpdateOptions updateOptions{};
  switch (request->update_type()) {
    case client::UpdateRowRequest_UPDATE_TYPE_REPLACE:
      updateOptions = UpdateOptions::REPLACE;
      break;
    case client::UpdateRowRequest_UPDATE_TYPE_MERGE:
      updateOptions = UpdateOptions::MERGE;
      break;
    default:
      assert(false && "invalid update type");
  }

  queryExecutor_->update(internalTable, updateOptions);

  return client::UpdateRowResponse{};
}

client::TableRows Db::scanDatabase(const client::ScanTableRequest* request) {
  auto table = schemaStore_->getTable(request->database_name().name(),
                                      request->table_name().name());
  assert(table.has_value() && "table not found");

  auto tableSchema =
      std::make_shared<TableSchema>(std::make_shared<Table>(table.value()));

  std::vector<TableSchemaType::ColumnIdType> colIds;
  for (const auto& colName : request->return_columns()) {
    const auto colId = tableSchema->getColumnId(colName.name());
    colIds.emplace_back(colId);
  }

  QueryPlan queryPlan{
      .operation = QueryOperation::TableScan,
      .schema = tableSchema,
      .outputColumns = std::move(colIds),
      .condition = request->condition(),
  };

  QueryPlanner queryPlanner{queryPlan, queryExecutor_};
  auto executableQueryPlan =
      queryPlanner.plan(InternalTable{.schema = tableSchema});
  auto result = QueryPlanner::execute(executableQueryPlan);
  return Transformer::from(result);
}

}  // namespace rk::projects::mydb