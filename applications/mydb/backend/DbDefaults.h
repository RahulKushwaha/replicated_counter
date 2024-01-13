//
// Created by Rahul  Kushwaha on 1/12/24.
//

#pragma once
#include "applications/mydb/backend/proto/db.pb.h"

namespace rk::projects::mydb {

struct TableDefaultColumns {
  static constexpr char* LOCK_MODE_NAME = "__internal__lock_mode";
  static constexpr std::uint32_t LOCK_MODE_COL_ID = 1001;
};

std::vector<internal::Column> getTableDefaultColumns();

}  // namespace rk::projects::mydb