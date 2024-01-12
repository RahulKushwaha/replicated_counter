//
// Created by Rahul  Kushwaha on 1/12/24.
//

#pragma once
#include "applications/mydb/backend/proto/db.pb.h"

namespace rk::projects::mydb {

struct TableDefaultColumns {
  static constexpr std::string LOCK_MODE = "__internal__lock_mode";
};

std::vector<internal::Column> getTableDefaultColumns() {
  std::vector<internal::Column> defaultColumns{};

  internal::Column lockModeCol{};
  lockModeCol.set_name(TableDefaultColumns::LOCK_MODE);
  lockModeCol.set_column_type(internal::Column_COLUMN_TYPE_INT64);

  defaultColumns.emplace_back(std::move(lockModeCol));

  return defaultColumns;
}

}  // namespace rk::projects::mydb