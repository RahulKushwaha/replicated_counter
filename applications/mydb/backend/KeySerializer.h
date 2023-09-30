//
// Created by Rahul  Kushwaha on 4/3/23.
//

#pragma once
#include "TableRow.h"
#include "applications/mydb/backend/proto/db.pb.h"

#include <ostream>
#include <sstream>

namespace rk::projects::mydb::prefix {

constexpr char DEFAULT_ESCAPE_CHARACTER = '/';

struct KeyFragments {
  struct Index {
    TableSchemaType::TableIdType indexId;
    std::vector<std::string> values;
  };

  TableSchemaType::DbIdType dbId;
  TableSchemaType::TableIdType tableId;
  std::optional<Index> primaryIndex;
  std::optional<TableSchemaType::ColumnIdType> colId;
  std::optional<Index> secondaryIndex;
  friend std::ostream& operator<<(std::ostream& os,
                                  const KeyFragments& fragments);
};

KeyFragments parseKey(const internal::Table& table, const std::string& key);

std::string parse(const std::string& str, std::size_t& startIndex,
                  char escapeCharacter = DEFAULT_ESCAPE_CHARACTER);

std::string escapeString(const std::string& input,
                         char escapeCharacter = DEFAULT_ESCAPE_CHARACTER);

std::string primaryKey(const internal::Table& table,
                       const std::vector<ColumnValue>& values);

std::string minimumIndexKey(const internal::Table& table,
                            TableSchemaType::TableIdType indexId);

std::string maximumIndexKey(const internal::Table& table,
                            TableSchemaType::TableIdType indexId,
                            std::vector<ColumnValue> values);

std::string maximumSecondaryIndexKey(
    const internal::Table& table, TableSchemaType::TableIdType indexId,
    std::vector<ColumnValue> primaryKeyValues,
    std::vector<ColumnValue> secondaryKeyValues);

std::string columnKey(const std::string& primaryKey, std::uint32_t colId);

std::string secondaryIndexKey(const internal::Table& table,
                              std::uint32_t indexId,
                              const std::vector<ColumnValue>& values,
                              const std::vector<ColumnValue>& primaryvalues);

}  // namespace rk::projects::mydb::prefix