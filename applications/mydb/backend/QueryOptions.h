//
// Created by Rahul  Kushwaha on 4/15/23.
//

#pragma once

#include "Common.h"

namespace rk::projects::mydb {

enum class InsertOptions {
  REPLACE,
  MERGE
};

enum class ScanDirection {
  FORWARD,
  BACKWARD,
};

struct ScanOptions {
  ScanDirection direction;
  std::int32_t maxRowsReturnSize;
};

struct IndexQueryOptions {
  TableSchemaType::TableIdType indexId;
  ScanDirection direction;
  std::vector<ColumnValue> startFromKey;
  std::int32_t maxRowsReturnSize;
};

}