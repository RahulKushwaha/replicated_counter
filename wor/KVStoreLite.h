//
// Created by Rahul  Kushwaha on 6/25/23.
//
#pragma once

#include "folly/experimental/coro/Task.h"

namespace rk::projects::wor {

class KVStoreLite {
public:
  virtual folly::coro::Task<bool> put(std::string key, std::string value) = 0;
  virtual folly::coro::Task<bool> putIfNotExists(std::string key,
                                                 std::string value) = 0;
  virtual folly::coro::Task<std::optional<std::string>>
  get(std::string key) = 0;
  virtual folly::coro::Task<bool> deleteRange(std::string start,
                                              std::string end) = 0;
};

} // namespace rk::projects::wor
