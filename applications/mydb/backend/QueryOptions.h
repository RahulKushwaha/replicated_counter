//
// Created by Rahul  Kushwaha on 4/15/23.
//

#pragma once

#include "Common.h"

namespace rk::projects::mydb {

enum class InsertOptions { REPLACE, MERGE };

enum class ScanDirection {
  FORWARD,
  BACKWARD,
};

struct IndexQueryOptions {
  TableSchemaType::TableIdType indexId;
  ScanDirection direction;
};

} // namespace rk::projects::mydb