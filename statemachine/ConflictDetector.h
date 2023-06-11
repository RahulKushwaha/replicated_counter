//
// Created by Rahul  Kushwaha on 6/10/23.
//
#pragma once
#include "statemachine/Common.h"
#include <map>
#include <set>
#include <string>
#include <vector>

namespace rk::projects::state_machine {
using LogId = std::int64_t;

class ConflictDetector {
public:
  void addKeySet(LogId logId, std::vector<std::string> keys) {
    std::set<std::string> modifiedKeys{keys.begin(), keys.end()};
    keysModifiedSet_.emplace(logId, std::move(modifiedKeys));
  }

  bool conflicted(SpeculativeExecution &speculativeExecution) {
    auto itr = keysModifiedSet_.upper_bound(speculativeExecution.logId);

    for (; itr != keysModifiedSet_.end(); itr++) {
      for (auto &key : speculativeExecution.keySet) {
        if (itr->second.find(key) != itr->second.end()) {
          return true;
        }
      }
    }

    return false;
  }

private:
  std::map<LogId, std::set<std::string>> keysModifiedSet_;
};

} // namespace rk::projects::state_machine
