//
// Created by rahul on 12/25/23.
//

#include "applications/mydb/backend/Db.h"

#include "applications/mydb/backend/QueryPlan.h"
#include "applications/mydb/backend/QueryPlanner.h"
#include "applications/mydb/backend/RequestTransformer.h"

namespace rk::projects::mydb {

Db::Db(std::shared_ptr<SchemaStore> schemaStore,
       std::shared_ptr<RocksReaderWriter> rocks)
    : schemaStore_{std::move(schemaStore)},
      queryExecutor_{std::make_shared<QueryExecutor>(std::move(rocks))} {}

google::protobuf::Empty Db::addDatabase(
    const client::AddDatabaseRequest* request) {
  auto table = schemaStore_->getTable("meta", "meta_db");
  assert(table.has_value() && "table not found");

  auto tableSchema =
      std::make_shared<TableSchema>(std::make_shared<Table>(table.value()));

  auto internalTable = Transformer::from(*request, tableSchema);

  queryExecutor_->insert(internalTable);

  return google::protobuf::Empty{};
}

google::protobuf::Empty Db::addTable(const client::AddTableRequest* request) {
  const auto db = request->database();
  auto table = schemaStore_->getTable(db.name().name(), "meta_db_table");
  assert(table.has_value() && "table not found");

  auto tableSchema =
      std::make_shared<TableSchema>(std::make_shared<Table>(table.value()));
  auto internalTable = Transformer::from(*request, tableSchema);

  queryExecutor_->insert(internalTable);

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

  return client::AddRowResponse{};
}

client::UpdateRowResponse Db::updateRow(
    const client::UpdateRowRequest* request) {
  return client::UpdateRowResponse{};
}

client::TableRows Db::scanDatabase(const client::ScanTableRequest* request) {
  auto table = schemaStore_->getTable("meta", "meta_db");
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
  auto result = queryPlanner.execute(executableQueryPlan);
  return Transformer::from(result);
}

}  // namespace rk::projects::mydb