//
// Created by Rahul  Kushwaha on 1/12/24.
//

#include "applications/mydb/backend/DbDefaults.h"

namespace rk::projects::mydb {

std::vector<internal::Column> getTableDefaultColumns() {
  std::vector<internal::Column> defaultColumns{};

  internal::Column lockModeCol{};
  lockModeCol.set_name(TableDefaultColumns::LOCK_MODE_NAME);
  lockModeCol.set_column_type(internal::Column_COLUMN_TYPE_INT64);
  lockModeCol.set_id(TableDefaultColumns::LOCK_MODE_COL_ID);

  defaultColumns.emplace_back(std::move(lockModeCol));

  return defaultColumns;
}

}  // namespace rk::projects::mydb