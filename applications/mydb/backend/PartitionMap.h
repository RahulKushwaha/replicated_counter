//
// Created by Rahul  Kushwaha on 1/17/24.
//
#pragma once
#include "applications/mydb/include/Db.h"

#include <map>
#include <string>

namespace rk::projects::mydb {

class PartitionMap {
 public:
  bool addTableToPartition(const std::string& partitionId,
                           const std::string& tableName) {
    auto itr = lookup_.emplace(partitionId, tableName);
    return itr.second;
  }

  bool addPartitionClient(const std::string& partitionId,
                          std::shared_ptr<Db> db) {
    auto result = dbClientLookup_.emplace(partitionId, std::move(db));
    return result.second;
  }

  std::optional<std::string> getPartition(const std::string& tableName) {
    if (auto itr = lookup_.find(tableName); itr != lookup_.end()) {
      return itr->second;
    }

    return {};
  }

  std::optional<std::shared_ptr<Db>> getPartitionClient(
      const std::string& partitionId) {
    if (auto itr = dbClientLookup_.find(partitionId);
        itr != dbClientLookup_.end()) {
      return itr->second;
    }

    return {};
  }

 private:
  std::map<std::string, std::string> lookup_;
  std::map<std::string, std::shared_ptr<Db>> dbClientLookup_;
};

}  // namespace rk::projects::mydb