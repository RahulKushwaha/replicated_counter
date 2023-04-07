//
// Created by Rahul  Kushwaha on 4/7/23.
//
#pragma once
#include <cstdint>
#include <string>
#include <sstream>
#include "folly/Conv.h"

namespace rk::projects::mydb::test_utils {
struct TableSchemaOutput {
  std::uint32_t dbId;
  std::uint32_t tableId;
  std::uint32_t indexId;
};

TableSchemaOutput parse(const std::string &key);
}