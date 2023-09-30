//
// Created by Rahul  Kushwaha on 4/3/23.
//

#pragma once
#include "Common.h"

#include <arrow/array.h>
#include <arrow/table.h>

#include <cstdint>
#include <string>
#include <variant>

namespace rk::projects::mydb {

using ColumnValue = std::variant<std::int64_t, std::string>;

std::string toString(const ColumnValue& rowValue);

struct TableColumnKeyValue {
  TableSchemaType::ColumnIdType id;
  ColumnValue value;
};

struct TableRow {
  std::vector<TableColumnKeyValue> columnKeyValues;
};

struct RawTableRow {
  using Key = std::string;
  using Value = std::string;
  using KeyValue = std::pair<Key, Value>;
  std::vector<KeyValue> keyValues;

  std::string toString() {
    std::stringstream ss;
    for (const auto& [k, v] : keyValues) {
      ss << k << " " << v << "\n";
    }

    return ss.str();
  }
};

struct InternalTable {
  std::shared_ptr<TableSchema> schema;
  std::shared_ptr<arrow::Table> table;
};

}  // namespace rk::projects::mydb