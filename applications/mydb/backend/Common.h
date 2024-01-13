//
// Created by Rahul  Kushwaha on 4/7/23.
//

#pragma once
#include "applications/mydb/backend/proto/db.pb.h"

#include <glog/logging.h>

namespace rk::projects::mydb {
using namespace internal;

template <typename Enumeration>
auto enumToInteger(Enumeration const value) ->
    typename std::underlying_type<Enumeration>::type {
  return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

struct TableSchemaType {
  using DbIdType = decltype(Database{}.id());
  using TableIdType = decltype(Table{}.id());
  using ColumnIdType = decltype(Column{}.id());
};

enum class LockType : std::uint32_t {
  NO_LOCK = 0,
  WRITE_LOCK = 1,
  READ_LOCK = 2,
};

namespace {
const std::unordered_set<TableSchemaType::TableIdType> EMPTY{};
}

class TableSchema {
 public:
  explicit TableSchema(std::shared_ptr<Table> table)
      : table_{std::move(table)} {
    for (auto& col : table_->columns()) {
      std::string_view columnName = col.name();
      columnIdToColLookup_[col.id()] = col;
      columnNameToColLookup_[columnName] = col;
    }

    for (auto& col : table_->primary_key_index().column_ids()) {
      primaryKeyColumns_.insert(col);
    }

    for (auto& idx : table_->secondary_index()) {
      for (auto& col : idx.column_ids()) {
        secondaryKeyColumns_[idx.id()].insert(col);
        colIdToSecondaryIdx_[col].insert(idx.id());
      }

      indexLookup_[idx.id()] = idx;
    }
  }

  inline TableSchemaType::ColumnIdType getColumnId(const std::string& colName) {
    return columnNameToColLookup_.at(colName).id();
  }

  inline std::string_view getColumnName(TableSchemaType::ColumnIdType colId) {
    return columnIdToColLookup_.at(colId).name();
  }

  inline internal::Column& getColumn(TableSchemaType::ColumnIdType colId) {
    return columnIdToColLookup_.at(colId);
  }

  inline Table& rawTable() { return *table_; }

  inline bool isPrimaryKeyColumn(const std::string& colName) {
    auto colItr = columnNameToColLookup_.find(colName);
    if (colItr == columnNameToColLookup_.end()) {
      return false;
    }

    return isPrimaryKeyColumn(colItr->second.id());
  }

  inline bool isPrimaryKeyColumn(TableSchemaType::ColumnIdType colId) {
    auto itr = primaryKeyColumns_.find(colId);
    return itr != primaryKeyColumns_.end();
  }

  inline bool isSecondIndexColumn(TableSchemaType::ColumnIdType idxId,
                                  TableSchemaType::ColumnIdType colId) {
    auto idxItr = secondaryKeyColumns_.find(idxId);
    if (idxItr == secondaryKeyColumns_.end()) {
      return false;
    }

    auto itr = idxItr->second.find(colId);
    return itr != idxItr->second.end();
  }

  inline const std::unordered_set<TableSchemaType::ColumnIdType>& indexIds(
      TableSchemaType::ColumnIdType colId) {
    auto idxItr = colIdToSecondaryIdx_.find(colId);
    if (idxItr != colIdToSecondaryIdx_.end()) {
      return idxItr->second;
    }

    return EMPTY;
  }

  inline std::optional<internal::Column_COLUMN_TYPE> getColumnType(
      TableSchemaType::ColumnIdType colId) {
    auto colItr = columnIdToColLookup_.find(colId);
    if (colItr != columnIdToColLookup_.end()) {
      return colItr->second.column_type();
    }

    return {};
  }

  inline SecondaryIndex& getSecondaryIndexById(
      TableSchemaType::TableIdType idxId) {
    return indexLookup_.at(idxId);
  }

 private:
  std::shared_ptr<Table> table_;
  std::unordered_map<TableSchemaType::ColumnIdType, internal::Column>
      columnIdToColLookup_;
  std::unordered_map<std::string_view, internal::Column> columnNameToColLookup_;
  std::unordered_map<TableSchemaType::TableIdType, internal::SecondaryIndex>
      indexLookup_;
  std::unordered_set<TableSchemaType::ColumnIdType> primaryKeyColumns_;
  std::unordered_map<TableSchemaType::ColumnIdType,
                     std::unordered_set<TableSchemaType::ColumnIdType>>
      secondaryKeyColumns_;
  std::unordered_map<TableSchemaType::ColumnIdType,
                     std::unordered_set<TableSchemaType::TableIdType>>
      colIdToSecondaryIdx_;
};

}  // namespace rk::projects::mydb