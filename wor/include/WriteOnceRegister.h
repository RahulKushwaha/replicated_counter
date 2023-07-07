//
// Created by Rahul  Kushwaha on 6/3/23.
//

#pragma once
#include <cstdint>
#include <optional>
#include <string>

namespace rk::projects::wor {

using LockId = std::int64_t;
template <typename T> using coro = folly::coro::Task<T>;

class WriteOnceRegister {
public:
  enum class ReadError {
    UNKNOWN,
    NOT_WRITTEN,
  };

  virtual coro<std::optional<LockId>> lock() = 0;
  virtual coro<bool> write(LockId lockId, std::string payload) = 0;
  virtual coro<std::variant<std::string, ReadError>> read() = 0;
};

} // namespace rk::projects::wor