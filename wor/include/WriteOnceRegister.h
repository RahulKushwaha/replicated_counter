//
// Created by Rahul  Kushwaha on 6/3/23.
//

#pragma once
#include <optional>
#include <cstdint>
#include <string>

namespace rk::projects::wor {

using LockId = std::int64_t;

class WriteOnceRegister {
 public:
  enum class ReadError {
    UNKNOWN,
    NOT_WRITTEN,
  };

  virtual std::optional<LockId> lock() = 0;
  virtual bool write(LockId lockId, std::string payload) = 0;
  virtual std::variant<std::string, ReadError> read() = 0;
};

}