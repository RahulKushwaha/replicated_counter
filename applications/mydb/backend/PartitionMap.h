//
// Created by Rahul  Kushwaha on 1/17/24.
//
#pragma once
#include <map>
#include <string>

namespace rk::projects::mydb {

class PartitionMap {
 public:
  bool addTableToPartition(const std::string& shardId,
                       const std::string& tableName) {
    if (auto itr = lookup_.find(tableName); itr != lookup_.end()) {
      return false;
    }

    return true;
  }

  std::optional<std::string> getPartition(const std::string& tableName) {
    if (auto itr = lookup_.find(tableName); itr != lookup_.end()) {
      return itr->second;
    }

    return {};
  }

 private:
  std::map<std::string, std::string> lookup_;
};

}  // namespace rk::projects::mydb