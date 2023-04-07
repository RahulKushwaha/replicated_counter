//
// Created by Rahul  Kushwaha on 4/3/23.
//

#pragma once
#include <sstream>

#include "TableRow.h"
#include "applications/mydb/backend/proto/db.pb.h"

namespace rk::projects::mydb::prefix {

constexpr char DEFAULT_ESCAPE_CHARACTER = '/';

std::string escapeString(const std::string &input,
                         char escapeCharacter = DEFAULT_ESCAPE_CHARACTER);

std::string
primaryKey(const internal::Table &table,
           const std::vector<ColumnValue> &values);

std::string
columnKey(const std::string &primaryKey, std::uint32_t colId);

std::string
secondaryIndexKey(const internal::Table &table,
                  std::uint32_t indexId,
                  const std::vector<ColumnValue> &values);

}