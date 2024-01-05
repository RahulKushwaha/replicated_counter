//
// Created by Rahul  Kushwaha on 4/7/23.
//

#pragma once
#include "Common.h"

#include <unordered_map>
#include <utility>

namespace rk::projects::mydb {

class SchemaStore {
 public:
  bool registerDb(const std::string& dbName, internal::Database database) {
    auto itr = lookup_.find(dbName);
    if (itr != lookup_.end()) {
      return false;
    }

    auto& [dbStore, tableLookup] = lookup_[dbName];

    lookup_[dbName].database.CopyFrom(database);
    return true;
  }

  bool registerTable(const std::string& dbName, const std::string& tableName,
                     internal::Table table) {
    if (getTable(dbName, tableName)) {
      return false;
    }

    lookup_[dbName].tableLookup[tableName] = std::move(table);
    return true;
  }

  std::optional<internal::Database> getDatabase(const std::string& dbName) {
    if (auto dbItr = lookup_.find(dbName); dbItr != lookup_.end()) {
      return dbItr->second.database;
    }

    return {};
  }

  std::optional<internal::Table> getTable(const std::string& dbName,
                                          const std::string& tableName) {
    if (auto dbItr = lookup_.find(dbName); dbItr != lookup_.end()) {
      const auto& tables = dbItr->second.tableLookup;
      if (auto tableItr = dbItr->second.tableLookup.find(tableName);
          tableItr != tables.end()) {
        return tableItr->second;
      }
    }

    return {};
  }

 private:
  struct DatabaseStore {
    internal::Database database;
    std::unordered_map<std::string, internal::Table> tableLookup;
  };

  std::unordered_map<std::string, DatabaseStore> lookup_;
};

}  // namespace rk::projects::mydb
