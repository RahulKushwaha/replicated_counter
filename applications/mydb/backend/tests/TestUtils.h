//
// Created by Rahul  Kushwaha on 4/7/23.
//
#pragma once
#include "applications/mydb/backend/TableRow.h"
#include "folly/Conv.h"
#include <arrow/array.h>
#include <arrow/builder.h>
//#include <arrow/compute/exec/exec_plan.h>
#include <arrow/dataset/dataset.h>
#include <arrow/table.h>
#include <cstdint>
#include <random>
#include <sstream>
#include <string>

namespace rk::projects::mydb::test_utils {
struct TableSchemaOutput {
  std::uint32_t dbId;
  std::uint32_t tableId;
  std::uint32_t indexId;
};

TableSchemaOutput parse(const std::string &key);

TableSchema createTableSchema(int numIntColumns = 5, int numStringColumns = 5,
                              int numColumnsInPrimaryIndex = 1,
                              int numSecondaryIndex = 2,
                              int numColumnsInSecondaryIndex = 1);

InternalTable getInternalTable(std::int32_t numRows = 10, int numIntColumns = 5,
                               int numStringColumns = 5,
                               int numColumnsInPrimaryIndex = 1,
                               int numSecondaryIndex = 2,
                               int numColumnsInSecondaryIndex = 1);

std::vector<ColumnValue> parsePrimaryKeyValues(InternalTable internalTable);

std::vector<ColumnValue> parseSecondaryKeyValues(InternalTable internalTable, int secondaryKeyIndex);

} // namespace rk::projects::mydb::test_utils