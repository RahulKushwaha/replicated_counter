//
// Created by Rahul  Kushwaha on 4/7/23.
//

#include <gtest/gtest.h>
#include <arrow/chunked_array.h>
#include "applications/mydb/backend/AddTableRowRequest.h"
#include "applications/mydb/backend/RowSerializer.h"
#include "applications/mydb/backend/tests/TestUtils.h"

namespace rk::projects::mydb {

TEST(RowSerializerTest, serializeInternalTable) {
  auto internalTable = test_utils::getInternalTable(1);
  auto rawTableRows = RowSerializer::serialize(internalTable);

  for (auto &row: rawTableRows) {
    LOG(INFO) << row.toString();
  }
  ASSERT_EQ(rawTableRows.size(), 10);
}

}