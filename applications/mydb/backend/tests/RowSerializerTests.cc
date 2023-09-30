//
// Created by Rahul  Kushwaha on 4/7/23.
//

#include "applications/mydb/backend/AddTableRowRequest.h"
#include "applications/mydb/backend/RowSerializer.h"
#include "applications/mydb/backend/tests/TestUtils.h"

#include <arrow/chunked_array.h>
#include <gtest/gtest.h>

namespace rk::projects::mydb {

TEST(RowSerializerTest, serializeInternalTable) {
  auto internalTable = test_utils::getInternalTable(10);
  auto rawTableRows = RowSerializer::serialize(internalTable);

  ASSERT_EQ(rawTableRows.size(), 10);
}

}  // namespace rk::projects::mydb