//
// Created by Rahul  Kushwaha on 9/6/23.
//

#include "QueryParser.h"

namespace rk::projects::mydb {
QueryParser::QueryParser(Request request,
                         std::shared_ptr<SchemaStore> schemaStore)
    : request_{std::move(request)}, schemaStore_{std::move(schemaStore)} {}

/**
 * Parse the @ScanTableRequest request and returns @QueryPlan
 * */
QueryPlan QueryParser::parse() const {
  std::string databaseName;
  std::string tableName;
  std::vector<std::string> outputColumnNames;
  auto queryOperation{QueryOperation::None};
  Condition condition;

  if (std::holds_alternative<client::ScanTableRequest>(request_)) {
    auto scanTableRequest = std::get<client::ScanTableRequest>(request_);
    databaseName = scanTableRequest.database_name().name();
    tableName = scanTableRequest.table_name().name();
    for (auto& returnColumnName : scanTableRequest.return_columns()) {
      outputColumnNames.emplace_back(returnColumnName.name());
    }

    queryOperation = QueryOperation::TableScan;
    condition = scanTableRequest.condition();
  } else if (std::holds_alternative<client::AddRowRequest>(request_)) {
    auto addRowRequest = std::get<client::AddRowRequest>(request_);
    databaseName = addRowRequest.database_name().name();
    tableName = addRowRequest.table_name().name();

    queryOperation = QueryOperation::Update;
  } else {
    assert(false && "unknown type present for parsing");
  }

  auto table = schemaStore_->getTable(databaseName, tableName);
  assert(table.has_value() && "table does not exist");

  auto tableSchema =
      std::make_shared<TableSchema>(std::make_shared<Table>(table.value()));

  std::vector<TableSchemaType::ColumnIdType> outputColumns;
  for (const auto& colName : outputColumnNames) {
    outputColumns.emplace_back(tableSchema->getColumnId(colName));
  }

  QueryPlan queryPlan{
      .operation = queryOperation,
      .schema = tableSchema,
      .outputColumns = std::move(outputColumns),
      .condition = std::move(condition),
  };

  return queryPlan;
}

}  // namespace rk::projects::mydb
