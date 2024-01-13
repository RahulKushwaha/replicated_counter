//
// Created by Rahul  Kushwaha on 1/12/24.
//
#pragma once
#include "fmt/format.h"

#include <stdexcept>
#include <string>

namespace rk::projects::mydb {

enum class ErrorCode {
  UNKNOWN = 0,
  UPDATE_CONDITION_FAILED = 1,
  ROW_ALREADY_EXISTS = 2,

  DATABASE_DOES_NOT_EXIST = 3,
  DATABASE_ALREADY_EXISTS = 4,

  TABLE_DOES_NOT_EXIST = 5,
  TABLE_ALREADY_EXISTS = 6,
};

template <typename Enumeration>
auto as_integer(Enumeration const value) ->
    typename std::underlying_type<Enumeration>::type {
  return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

struct DbError final : public std::runtime_error {
  explicit DbError(ErrorCode errorCode)
      : runtime_error(fmt::format("error encountered. error-code: {}",
                                  as_integer(errorCode))),
        errorCode_{errorCode} {}

  ErrorCode errorCode_;
};

inline DbError makeDbError(ErrorCode errorCode) {
  return DbError(errorCode);
}

}  // namespace rk::projects::mydb