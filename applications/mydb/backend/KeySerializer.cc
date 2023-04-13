//
// Created by Rahul  Kushwaha on 4/3/23.
//
#include <sstream>

#include "KeySerializer.h"
#include "TableRow.h"
#include "applications/mydb/backend/proto/db.pb.h"
#include "folly/Conv.h"

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

KeyFragments parseKey(const internal::Table &table, const std::string &key) {
  enum class TokenType {
    DB_ID,
    TBL_ID,
    INDEX_ID,
    PRIMARY_KEY_PARTS,
    COL_ID,
    SECONDARY_INDEX_KEY_PARTS,
  };

  TokenType current{TokenType::DB_ID};
  bool indexKeyPartsColId = true;
  KeyFragments keyFragments;

  auto length = key.size();
  std::size_t i = 0;
  while (i < length) {
    auto token = parse(key, i, DEFAULT_ESCAPE_CHARACTER);
    switch (current) {
      case TokenType::DB_ID: {
        keyFragments.dbId = folly::to<TableSchemaType::DbIdType>(token);
        current = TokenType::TBL_ID;
      }
        break;

      case TokenType::TBL_ID: {
        keyFragments.tableId = folly::to<TableSchemaType::TableIdType>(token);
        current = TokenType::INDEX_ID;
      }
        break;

      case TokenType::INDEX_ID: {
        auto indexId =
            folly::to<TableSchemaType::TableIdType>(token);
        // primary key index is always 0
        if (indexId == 0) {
          keyFragments.primaryIndex = {KeyFragments::Index{indexId}};
          current = TokenType::PRIMARY_KEY_PARTS;
        } else {
          keyFragments.secondaryIndex = {KeyFragments::Index{indexId}};
          current = TokenType::SECONDARY_INDEX_KEY_PARTS;
        }
      }
        break;

      case TokenType::PRIMARY_KEY_PARTS: {
        if (!indexKeyPartsColId) {
          keyFragments.primaryIndex->values.emplace_back(std::move(token));
        }

        indexKeyPartsColId = !indexKeyPartsColId;

        if (keyFragments.primaryIndex->values.size()
            == table.primary_key_index().column_ids().size()) {
          current = TokenType::COL_ID;
        }
      }
        break;

      case TokenType::COL_ID: {
        keyFragments.colId = {folly::to<TableSchemaType::ColumnIdType>(token)};
      }
        break;

      case TokenType::SECONDARY_INDEX_KEY_PARTS: {
        if (!indexKeyPartsColId) {
          keyFragments.secondaryIndex->values.emplace_back(std::move(token));
        }

        indexKeyPartsColId = !indexKeyPartsColId;
      }
        break;

      default:
        assert(false);
        break;
    }
  }

  return keyFragments;
}

std::string
parse(const std::string &str,
      std::size_t &startIndex,
      char escapeCharacter) {
  std::stringstream ss{};
  std::size_t length = str.size();
  for (std::size_t &i = startIndex; i < length; i++) {
    if (str[i] != escapeCharacter) {
      ss << str[i];
    } else if (str[i] == escapeCharacter
        && (i + 1 < length && str[i + 1] == escapeCharacter)) {
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