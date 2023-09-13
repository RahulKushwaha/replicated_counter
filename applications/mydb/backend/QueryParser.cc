//
// Created by Rahul  Kushwaha on 9/6/23.
//

#include "QueryParser.h"

namespace rk::projects::mydb {
QueryParser::QueryParser(client::ScanTableRequest scanTableRequest,
                         std::shared_ptr<SchemaStore> schemaStore)
    : scanTableRequest_{std::move(scanTableRequest)}, schemaStore_{std::move(
                                                          schemaStore)} {}

/**
 * Parse the @ScanTableRequest request and returns @QueryPlan
 * */
QueryPlan QueryParser::parse() {

  auto table = schemaStore_->getTable(scanTableRequest_.database_name().name(),
                                      scanTableRequest_.table_name().name());

  auto tableSchema =
      std::make_shared<TableSchema>(std::make_shared<Table>(table.value()));

  std::vector<TableSchemaType::ColumnIdType> outputColumns;
  for (const auto &col : scanTableRequest_.return_columns()) {
    outputColumns.emplace_back(tableSchema->getColumnId(col.name()));
  }

  QueryPlan queryPlan{
      .operation = QueryOperation::TableScan,
      .schema = tableSchema,
      .outputColumns = outputColumns,
      .condition = scanTableRequest_.condition(),
  };

  return queryPlan;
}

} // namespace rk::projects::mydb
