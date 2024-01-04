//
// Created by Rahul  Kushwaha on 4/15/23.
//

#pragma once

#include "Common.h"
#include "TableRow.h"

namespace rk::projects::mydb {

enum class UpdateOptions { REPLACE, MERGE };

enum class ScanDirection {
  FORWARD,
  BACKWARD,
};

struct IndexQueryOptions {
  TableSchemaType::TableIdType indexId;
  ScanDirection direction;
  std::vector<ColumnValue> secondaryKeyValues;
  std::vector<ColumnValue> primaryKeyValues;
  std::int32_t maxRowsReturnSize;
};

struct ScanOptions {
  ScanDirection direction;
  std::int32_t maxRowsReturnSize;
  std::string seekPosition;
  std::string prefix;
};

}  // namespace rk::projects::mydb