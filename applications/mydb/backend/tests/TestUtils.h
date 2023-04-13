//
// Created by Rahul  Kushwaha on 4/7/23.
//
#pragma once
#include <cstdint>
#include <string>
#include <sstream>
#include <random>
#include "folly/Conv.h"
#include "applications/mydb/backend/TableRow.h"
#include <arrow/array.h>
#include <arrow/builder.h>
#include <arrow/table.h>

namespace rk::projects::mydb::test_utils {
struct TableSchemaOutput {
  std::uint32_t dbId;
  std::uint32_t tableId;
  std::uint32_t indexId;
};

TableSchemaOutput parse(const std::string &key);

TableSchema createTableSchema(int numIntColumns = 5,
                              int numStringColumns = 5,
                              int numColumnsInPrimaryIndex = 1,
                              int numSecondaryIndex = 2,
                              int numColumnsInSecondaryIndex = 1);

InternalTable getInternalTable(std::int32_t numRows = 10,
                               int numIntColumns = 5,
                               int numStringColumns = 5,
                               int numColumnsInPrimaryIndex = 1,
                               int numSecondaryIndex = 2,
                               int numColumnsInSecondaryIndex = 1);

}