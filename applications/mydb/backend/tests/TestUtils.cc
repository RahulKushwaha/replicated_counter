//
// Created by Rahul  Kushwaha on 4/7/23.
//
#pragma once
#include "TestUtils.h"

namespace rk::projects::mydb::test_utils {

TableSchemaOutput parse(const std::string &key) {
  std::string str;
  std::stringstream ss(key);

  TableSchemaOutput schema{};

  int index = 0;
  while (std::getline(ss, str, '/') && index < 3) {
    switch (index) {
      case 0:
        schema.dbId = folly::to<uint32_t>(str);
        break;

      case 1:
        schema.tableId = folly::to<uint32_t>(str);
        break;

      case 2:
        schema.indexId = folly::to<uint32_t>(str);
        break;

      default:
        throw std::runtime_error{"unknown case"};
    }

    index++;
  }


  return schema;
}

}