//
// Created by Rahul  Kushwaha on 4/4/23.
//

#pragma once

#include <unordered_map>
#include <functional>
#include <string_view>
#include "applications/mydb/client/proto/db.pb.h"
#include "applications/mydb/backend/proto/db.pb.h"
#include "Common.h"

namespace rk::projects::mydb::internal {


struct AddTableRowRequest {
  using ClientColumnValueRef = std::reference_wrapper<client::ColumnValue>;
  using ClientColumnValues = decltype(client::AddRowRequest{}.mutable_column_values());

  std::shared_ptr<TableSchema> table;
  std::unordered_map<std::uint32_t, client::ColumnValue>
      columnNameToValueLookup;
  client::AddRowRequest addRowRequest;
};

}