//
// Created by Rahul  Kushwaha on 4/3/23.
//
#include <sstream>

#include "KeySerializer.h"
#include "TableRow.h"
#include "applications/mydb/backend/proto/db.pb.h"
#include "folly/Conv.h"

namespace rk::projects::mydb::prefix {

namespace {
enum class TokenType {
  DB_ID,
  TBL_ID,
  INDEX_ID,
  PRIMARY_KEY_PARTS,
  COL_ID,
  SECONDARY_INDEX_KEY_PARTS,
};

std::string_view to_string(TokenType tokenType) {
  switch (tokenType) {
    using enum TokenType;
  case DB_ID:
    return "DB_ID";
  case TBL_ID:
    return "TBL_ID";
  case INDEX_ID:
    return "INDEX_ID";
  case PRIMARY_KEY_PARTS:
    return "PRIMARY_KEY_PARTS";
  case COL_ID:
    return "COL_ID";
  case SECONDARY_INDEX_KEY_PARTS:
    return "SECONDARY_INDEX_KEY_PARTS";
  }
}
} // namespace

// This method add extra escape character
// text/text -> text//text
std::string escapeString(const std::string &input, char escapeCharacter) {
  std::stringstream ss;
  for (const auto c : input) {
    if (c == escapeCharacter) {
      ss << escapeCharacter << escapeCharacter;
    } else {
      ss << c;
    }
  }

  return ss.str();
}

KeyFragments parseKey(const internal::Table &table, const std::string &key) {
  TokenType current{TokenType::DB_ID};
  bool indexKeyPartsColId = true;
  KeyFragments keyFragments;
  internal::SecondaryIndex secondaryIndex;

  auto length = key.size();
  std::size_t i = 0;
  while (i < length) {

    auto token = parse(key, i, DEFAULT_ESCAPE_CHARACTER);

    switch (current) {
    case TokenType::DB_ID: {
      keyFragments.dbId = folly::to<TableSchemaType::DbIdType>(token);
      current = TokenType::TBL_ID;
    } break;

    case TokenType::TBL_ID: {
      keyFragments.tableId = folly::to<TableSchemaType::TableIdType>(token);
      current = TokenType::INDEX_ID;
    } break;

    case TokenType::INDEX_ID: {
      auto indexId = folly::to<TableSchemaType::TableIdType>(token);
      // primary key index is always 0
      if (indexId == 0) {
        keyFragments.primaryIndex = {KeyFragments::Index{indexId}};
        current = TokenType::PRIMARY_KEY_PARTS;
      } else {
        keyFragments.secondaryIndex = {KeyFragments::Index{indexId}};
        for (const auto &idx : table.secondary_index()) {
          if (idx.id() == indexId) {
            secondaryIndex = idx;
            break;
          }
        }
        current = TokenType::SECONDARY_INDEX_KEY_PARTS;
      }
    } break;

    case TokenType::PRIMARY_KEY_PARTS: {
      // Fetch all primary key and primary key value combination
      if (!indexKeyPartsColId) {
        keyFragments.primaryIndex->values.emplace_back(std::move(token));
      }

      indexKeyPartsColId = !indexKeyPartsColId;

      if (keyFragments.primaryIndex->values.size() ==
          table.primary_key_index().column_ids().size()) {
        current = TokenType::COL_ID;
      }
    } break;

    case TokenType::COL_ID: {
      keyFragments.colId = {folly::to<TableSchemaType::ColumnIdType>(token)};
    } break;

    case TokenType::SECONDARY_INDEX_KEY_PARTS: {
      if (!indexKeyPartsColId) {
        keyFragments.secondaryIndex->values.emplace_back(std::move(token));
      }

      indexKeyPartsColId = !indexKeyPartsColId;

      if (keyFragments.secondaryIndex->values.size() ==
          secondaryIndex.column_ids().size()) {
        keyFragments.primaryIndex = {KeyFragments::Index{0}};
        current = TokenType::PRIMARY_KEY_PARTS;
      }
    } break;

    default:
      assert(false);
      break;
    }
  }

  return keyFragments;
}

std::string parse(const std::string &str, std::size_t &startIndex,
                  char escapeCharacter) {
  std::stringstream ss{};
  std::size_t length = str.size();
  for (std::size_t &i = startIndex; i < length; i++) {
    if (str[i] != escapeCharacter) {
      ss << str[i];
    } else if (str[i] == escapeCharacter &&
               (i + 1 < length && str[i + 1] == escapeCharacter)) {
      ss << escapeCharacter;
      i++;
    } else {
      assert(str[i] == escapeCharacter);
      i++;
      break;
    }
  }

  return ss.str();
}

std::string primaryKey(const internal::Table &table,
                       const std::vector<ColumnValue> &values) {
  assert(table.primary_key_index().column_ids().size() == values.size());
  std::stringstream ss;
  ss << table.db().id() << DEFAULT_ESCAPE_CHARACTER << table.id()
     << DEFAULT_ESCAPE_CHARACTER << table.primary_key_index().id();

  for (std::int32_t index = 0; index < values.size(); index++) {
    ss << DEFAULT_ESCAPE_CHARACTER
       << table.primary_key_index().column_ids(index)
       << DEFAULT_ESCAPE_CHARACTER
       << escapeString(mydb::toString(values[index]));
  }

  return ss.str();
}

std::string minimumIndexKey(const internal::Table &table,
                            TableSchemaType::TableIdType indexId) {
  std::stringstream ss;
  ss << table.db().id() << DEFAULT_ESCAPE_CHARACTER << table.id()
     << DEFAULT_ESCAPE_CHARACTER << indexId;

  return ss.str();
}

std::string maximumIndexKey(const internal::Table &table,
                            TableSchemaType::TableIdType indexId,
                            std::vector<ColumnValue> values) {
  if (table.primary_key_index().id() == indexId) {
    std::stringstream ss;
    // Todo: AMAN Add the logic of replacing ZZZZZ  with something better.
    ss << primaryKey(table, values) << DEFAULT_ESCAPE_CHARACTER << "ZZZZZZ";

    return ss.str();
  }
  throw std::runtime_error{"i should not be here"};
}

std::string
maximumSecondaryIndexKey(const internal::Table &table,
                         TableSchemaType::TableIdType indexId,
                         std::vector<ColumnValue> primaryKeyValues,
                         std::vector<ColumnValue> secondaryKeyValues) {

  for (int i = 0; i < table.secondary_index_size(); i++) {
    if (indexId == table.secondary_index(i).id()) {
      std::stringstream ss;
      ss << secondaryIndexKey(table, indexId, secondaryKeyValues,
                              primaryKeyValues)
         << DEFAULT_ESCAPE_CHARACTER << "ZZZZZZ";
      LOG(INFO)<<ss.str();
      return ss.str();
    }
  }

  throw std::runtime_error{"i should not be here"};
}

std::string columnKey(const std::string &primaryKey, std::uint32_t colId) {
  std::stringstream ss;
  ss << primaryKey << DEFAULT_ESCAPE_CHARACTER << colId;
  return ss.str();
}

std::string secondaryIndexKey(const internal::Table &table,
                              std::uint32_t indexId,
                              const std::vector<ColumnValue> &values,
                              const std::vector<ColumnValue> &primaryValues) {
  for (const auto &secondaryIndex : table.secondary_index()) {
    if (secondaryIndex.id() == indexId) {
      assert(secondaryIndex.column_ids().size() == values.size());
      std::stringstream ss;
      ss << table.db().id() << DEFAULT_ESCAPE_CHARACTER << table.id()
         << DEFAULT_ESCAPE_CHARACTER << indexId;

      for (std::int32_t index = 0; index < values.size(); index++) {
        ss << DEFAULT_ESCAPE_CHARACTER << secondaryIndex.column_ids(index)
           << DEFAULT_ESCAPE_CHARACTER
           << escapeString(mydb::toString(values[index]));
      }

      for (std::int32_t index = 0; index < primaryValues.size(); index++) {
        ss << DEFAULT_ESCAPE_CHARACTER
           << table.primary_key_index().column_ids(index)
           << DEFAULT_ESCAPE_CHARACTER
           << escapeString(mydb::toString(primaryValues[index]));
      }

      return ss.str();
    }
  }

  assert(false);
}

std::ostream &operator<<(std::ostream &os, const KeyFragments &fragments) {
  os << "dbId: " << fragments.dbId << " tableId: " << fragments.tableId
     << " primaryIndex: "
     << (fragments.primaryIndex.has_value() ? "NON_EMPTY_PRIMARY_INDEX"
                                            : "EMPTY_PRIMARY_INDEX")
     << " colId: "
     << (fragments.colId.has_value() ? "NON_EMPTY_COL_ID" : "MPTY_COL_ID")
     << " secondaryIndex: "
     << (fragments.secondaryIndex.has_value() ? "NON_EMPTY_SEC_INDEX"
                                              : "EMPTY_SEC_INDEX");
  return os;
}

} // namespace rk::projects::mydb::prefix