//
// Created by Rahul  Kushwaha on 4/3/23.
//
#include <sstream>

#include "KeySerializer.h"
#include "TableRow.h"
#include "applications/mydb/backend/proto/db.pb.h"

namespace rk::projects::mydb::prefix {

std::string escapeString(const std::string &input, char escapeCharacter) {
  std::stringstream ss;
  for (const auto c: input) {
    if (c == escapeCharacter) {
      ss << escapeCharacter << escapeCharacter;
    } else {
      ss << c;
    }
  }

  return ss.str();
}

std::string
primaryKey(const internal::Table &table,
           const std::vector<ColumnValue> &values) {
  assert(table.primary_key_index().column_ids().size() == values.size());
  std::stringstream ss;
  ss << table.db().id() << DEFAULT_ESCAPE_CHARACTER
     << table.id() << DEFAULT_ESCAPE_CHARACTER
     << table.primary_key_index().id();

  for (std::int32_t index = 0; index < values.size(); index++) {
    ss << DEFAULT_ESCAPE_CHARACTER
       << table.primary_key_index().column_ids(index)
       << DEFAULT_ESCAPE_CHARACTER
       << escapeString(mydb::toString(values[index]));
  }

  return ss.str();
}

std::string
columnKey(const std::string &primaryKey, std::uint32_t colId) {
  std::stringstream ss;
  ss << primaryKey << DEFAULT_ESCAPE_CHARACTER << colId;
  return ss.str();
}

std::string
secondaryIndexKey(const internal::Table &table,
                  std::uint32_t indexId,
                  const std::vector<ColumnValue> &values) {
  for (const auto &secondaryIndex: table.secondary_index()) {
    if (secondaryIndex.id() == indexId) {
      assert(secondaryIndex.column_ids().size() == values.size());
      std::stringstream ss;
      ss << table.db().id() << DEFAULT_ESCAPE_CHARACTER
         << table.id() << DEFAULT_ESCAPE_CHARACTER
         << indexId;

      for (std::int32_t index = 0; index < values.size(); index++) {
        ss << DEFAULT_ESCAPE_CHARACTER
           << secondaryIndex.column_ids(index)
           << DEFAULT_ESCAPE_CHARACTER
           << escapeString(mydb::toString(values[index]));
      }

      return ss.str();
    }
  }

  assert(false);
}

}