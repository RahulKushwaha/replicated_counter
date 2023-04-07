//
// Created by Rahul  Kushwaha on 4/7/23.
//

#pragma once
#include <glog/logging.h>
#include "applications/mydb/backend/proto/db.pb.h"

namespace rk::projects::mydb {
using namespace internal;

struct TableSchemaType {
  using DbIdType = decltype(Database{}.id());
  using TableIdType = decltype(Table{}.id());
  using ColumnIdType = decltype(Column{}.id());
};

class TableSchema {
 public:
  explicit TableSchema(std::shared_ptr<Table> table)
      : table_{std::move(table)} {
    for (auto &col: table_->columns()) {
      std::string_view columnName = col.name();
      columnIdToName_[col.id()] = columnName;
      columnNameToId_[columnName] = col.id();
    }
  }
  inline TableSchemaType::ColumnIdType getColumnId(const std::string &colName) {
    return columnNameToId_.at(colName);
  }

  inline std::string_view getColumnName(TableSchemaType::ColumnIdType colId) {
    return columnIdToName_.at(colId);
  }

  inline Table &rawTable() {
    return *table_;
  }

 private:
  std::shared_ptr<Table> table_;
  std::unordered_map<TableSchemaType::ColumnIdType, std::string_view>
      columnIdToName_;
  std::unordered_map<std::string_view, TableSchemaType::ColumnIdType>
      columnNameToId_;
};

}