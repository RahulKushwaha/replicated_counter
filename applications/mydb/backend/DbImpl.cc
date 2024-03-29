//
// Created by rahul on 12/25/23.
//

#include "applications/mydb/backend/DbImpl.h"

#include "applications/mydb/backend/Errors.h"
#include "applications/mydb/backend/QueryPlan.h"
#include "applications/mydb/backend/QueryPlanner.h"
#include "applications/mydb/backend/RequestTransformer.h"
#include "applications/mydb/format/FormatTable.h"

namespace rk::projects::mydb {

DbImpl::DbImpl(std::shared_ptr<SchemaStore> schemaStore,
               std::shared_ptr<RocksReaderWriter> rocks)
    : schemaStore_{std::move(schemaStore)},
      queryExecutor_{std::make_shared<QueryExecutor>(std::move(rocks))},
      dbIds_{0},
      tableIds_{0} {}

coro<google::protobuf::Empty> DbImpl::addDatabase(
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

  co_return google::protobuf::Empty{};
}

coro<google::protobuf::Empty> DbImpl::addTable(
    const client::AddTableRequest* request) {
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

  co_return google::protobuf::Empty{};
}

coro<client::AddRowResponse> DbImpl::addRow(
    const client::AddRowRequest* request) {
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
  co_return response;
}

coro<client::UpdateRowResponse> DbImpl::updateRow(
    const client::UpdateRowRequest* request) {
  auto table = schemaStore_->getTable(request->database_name().name(),
                                      request->table_name().name());
  assert(table.has_value() && "table not found");

  auto tableSchema =
      std::make_shared<TableSchema>(std::make_shared<Table>(table.value()));

  auto conditionWithLockCheck =
      addConditionToCheckWriteLock(request->condition());

  QueryPlan queryPlan{
      .operation = QueryOperation::TableScan,
      .schema = tableSchema,
      .condition = std::move(conditionWithLockCheck),
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

  co_return client::UpdateRowResponse{};
}

coro<client::GetRowResponse> DbImpl::getRow(
    const client::GetRowRequest* request) {
  auto table = schemaStore_->getTable(request->database_name().name(),
                                      request->table_name().name());
  assert(table.has_value() && "table not found");

  auto tableSchema =
      std::make_shared<TableSchema>(std::make_shared<Table>(table.value()));

  auto internalTable = Transformer::from(*request, tableSchema);
  auto response = queryExecutor_->get(internalTable);
  auto tableValues = Transformer::from(response);

  client::GetRowResponse rowResponse{};
  *rowResponse.mutable_table_rows() = std::move(tableValues);

  co_return rowResponse;
}

coro<client::MultiTableOperationResponse> DbImpl::multiTableOperation(
    const client::MultiTableOperationRequest* request) {
  client::MultiTableOperationResponse response{};

  for (auto& singleTableRequest : request->single_table_operation_request()) {
    client::SingleTableOperationResponse singleTableOperationResponse{};

    if (singleTableRequest.has_add_row_request()) {
      auto addRowResponse =
          co_await addRow(&singleTableRequest.add_row_request());
      *singleTableOperationResponse.mutable_add_row_response() =
          std::move(addRowResponse);
    } else if (singleTableRequest.has_update_row_request()) {
      auto updateRowResponse =
          co_await updateRow(&singleTableRequest.update_row_request());

      *singleTableOperationResponse.mutable_update_row_response() =
          std::move(updateRowResponse);
    } else if (singleTableRequest.has_get_row_request()) {
      auto getRowResponse =
          co_await getRow(&singleTableRequest.get_row_request());

      *singleTableOperationResponse.mutable_get_row_response() =
          std::move(getRowResponse);
    }

    response.mutable_single_table_operation_response()->Add(
        std::move(singleTableOperationResponse));
  }

  co_return response;
}

coro<client::ScanTableResponse> DbImpl::scanTable(
    const client::ScanTableRequest* request) {
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
  auto tableRows = Transformer::from(result);

  client::ScanTableResponse response{};
  *(response.mutable_table_rows()) = tableRows;

  co_return response;
}

Condition DbImpl::addConditionToCheckWriteLock(Condition condition) {
  BinaryCondition binaryCondition{};
  binaryCondition.set_op(LogicalOperator::AND);

  UnaryCondition checkLock{};
  auto intCondition = checkLock.mutable_int_condition();
  intCondition->set_col_name(TableDefaultColumns::LOCK_MODE_NAME);
  intCondition->set_op(IntCondition_Operation_EQ);
  intCondition->set_value(enumToInteger(LockType::NO_LOCK));

  Condition rhsCondition{};
  *rhsCondition.mutable_unary_condition() = std::move(checkLock);

  *binaryCondition.mutable_c1() = std::move(condition);
  *binaryCondition.mutable_c2() = std::move(rhsCondition);

  Condition returnCondition{};
  *returnCondition.mutable_binary_condition() = std::move(binaryCondition);
  return returnCondition;
}

coro<client::PrepareTransactionResponse> DbImpl::prepareTransaction(
    const client::PrepareTransactionRequest* request) {
  throw std::runtime_error{"METHOD_NOT_IMPLEMENTED"};
}

coro<client::CommitTransactionResponse> DbImpl::commitTransaction(
    const client::CommitTransactionRequest* request) {
  throw std::runtime_error{"METHOD_NOT_IMPLEMENTED"};
}

coro<client::AbortTransactionResponse> DbImpl::abortTransaction(
    const client::AbortTransactionRequest* request) {
  throw std::runtime_error{"METHOD_NOT_IMPLEMENTED"};
}

}  // namespace rk::projects::mydb