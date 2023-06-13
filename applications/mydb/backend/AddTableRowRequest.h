//
// Created by Rahul  Kushwaha on 4/4/23.
//

#pragma once

#include "Common.h"
#include "applications/mydb/backend/proto/db.pb.h"
#include "applications/mydb/client/proto/db.pb.h"
#include <functional>
#include <string_view>
#include <unordered_map>

namespace rk::projects::mydb::internal {

struct AddTableRowRequest {
  using ClientColumnValueRef = std::reference_wrapper<client::ColumnValue>;
  using ClientColumnValues =
      decltype(client::AddRowRequest{}.mutable_column_values());

  std::shared_ptr<TableSchema> table;
  std::unordered_map<std::uint32_t, client::ColumnValue>
      columnNameToValueLookup;
  client::AddRowRequest addRowRequest;
};

} // namespace rk::projects::mydb::internal