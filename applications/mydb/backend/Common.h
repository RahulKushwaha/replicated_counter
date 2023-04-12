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

namespace {
const std::unordered_set<TableSchemaType::TableIdType> EMPTY{};
}

class TableSchema {
 public:
  explicit TableSchema(std::shared_ptr<Table> table)
      : table_{std::move(table)} {
    for (auto &col: table_->columns()) {
      std::string_view columnName = col.name();
      columnIdToName_[col.id()] = columnName;
      columnNameToId_[columnName] = col.id();
    }

    for (auto &col: table_->primary_key_index().column_ids()) {
      primaryKeyColumns_.insert(col);
    }

    for (auto &idx: table_->secondary_index()) {
      for (auto &col: idx.column_ids()) {
        secondaryKeyColumns_[idx.id()].insert(col);
        colIdToSecondaryIdx_[col].insert(idx.id());
      }
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

  inline bool isPrimaryKeyColumn(const std::string &colName) {
    auto colItr = columnNameToId_.find(colName);
    if (colItr == columnNameToId_.end()) {
      return false;
    }

    return isPrimaryKeyColumn(colItr->second);
  }

  inline bool isPrimaryKeyColumn(TableSchemaType::ColumnIdType colId) {
    auto itr = primaryKeyColumns_.find(colId);
    return itr != primaryKeyColumns_.end();
  }

  inline bool
  isSecondIndexColumn(TableSchemaType::ColumnIdType idxId,
                      TableSchemaType::ColumnIdType colId) {
    auto idxItr = secondaryKeyColumns_.find(idxId);
    if (idxItr == secondaryKeyColumns_.end()) {
      return false;
    }

    auto itr = idxItr->second.find(colId);
    return itr != idxItr->second.end();
  }

  inline const std::unordered_set<TableSchemaType::ColumnIdType> &
  indexIds(TableSchemaType::ColumnIdType colId) {
    auto idxItr = colIdToSecondaryIdx_.find(colId);
    if (idxItr != colIdToSecondaryIdx_.end()) {
      return idxItr->second;
    }

    return EMPTY;
  }

  inline internal::Column_COLUMN_TYPE
  getColumnType(TableSchemaType::ColumnIdType colId) {
    return internal::Column_COLUMN_TYPE_INT64;
  }

 private:
  std::shared_ptr<Table> table_;
  std::unordered_map<TableSchemaType::ColumnIdType, std::string_view>
      columnIdToName_;
  std::unordered_map<std::string_view, TableSchemaType::ColumnIdType>
      columnNameToId_;
  std::unordered_set<TableSchemaType::ColumnIdType> primaryKeyColumns_;
  std::unordered_map<TableSchemaType::ColumnIdType,
                     std::unordered_set<TableSchemaType::ColumnIdType >>
      secondaryKeyColumns_;
  std::unordered_map<TableSchemaType::ColumnIdType,
                     std::unordered_set<TableSchemaType::TableIdType>>
      colIdToSecondaryIdx_;
};

}