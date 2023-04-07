//
// Created by Rahul  Kushwaha on 4/3/23.
//

#pragma once
#include <variant>
#include <string>
#include <cstdint>
#include "Common.h"

namespace rk::projects::mydb {

using ColumnValue = std::variant<std::int64_t, std::string>;

std::string toString(const ColumnValue &rowValue);

struct TableColumnKeyValue {
  TableSchemaType::ColumnIdType id;
  ColumnValue value;
};

struct TableRow {
  std::vector<TableColumnKeyValue> columnKeyValues;
};

struct RawTableRow {
  using KeyValue = std::pair<std::string, std::string>;
  std::vector<KeyValue> keyValues;
};

}