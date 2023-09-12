//
// Created by Rahul  Kushwaha on 9/12/23.
//
#include "applications/mydb/backend/tests/TestUtils.h"
#include "applications/mydb/format/FormatTable.h"
#include <gtest/gtest.h>

namespace rk::projects::mydb {

namespace {
const char *output =
    R"(+----------------+----------------+----------------+------------------------+------------------------+------------------------+
| col_int_name_1 | col_int_name_2 | col_int_name_3 |     col_str_name_4     |     col_str_name_5     |     col_str_name_6     |
+----------------+----------------+----------------+------------------------+------------------------+------------------------+
|              0 |             10 |             20 | col_str_name_4-value-0 | col_str_name_5-value-0 | col_str_name_6-value-0 |
+----------------+----------------+----------------+------------------------+------------------------+------------------------+
|              1 |             11 |             21 | col_str_name_4-value-1 | col_str_name_5-value-1 | col_str_name_6-value-1 |
+----------------+----------------+----------------+------------------------+------------------------+------------------------+
|              2 |             12 |             22 | col_str_name_4-value-2 | col_str_name_5-value-2 | col_str_name_6-value-2 |
+----------------+----------------+----------------+------------------------+------------------------+------------------------+
|              3 |             13 |             23 | col_str_name_4-value-3 | col_str_name_5-value-3 | col_str_name_6-value-3 |
+----------------+----------------+----------------+------------------------+------------------------+------------------------+
|              4 |             14 |             24 | col_str_name_4-value-4 | col_str_name_5-value-4 | col_str_name_6-value-4 |
+----------------+----------------+----------------+------------------------+------------------------+------------------------+
|              5 |             15 |             25 | col_str_name_4-value-5 | col_str_name_5-value-5 | col_str_name_6-value-5 |
+----------------+----------------+----------------+------------------------+------------------------+------------------------+
|              6 |             16 |             26 | col_str_name_4-value-6 | col_str_name_5-value-6 | col_str_name_6-value-6 |
+----------------+----------------+----------------+------------------------+------------------------+------------------------+
|              7 |             17 |             27 | col_str_name_4-value-7 | col_str_name_5-value-7 | col_str_name_6-value-7 |
+----------------+----------------+----------------+------------------------+------------------------+------------------------+
|              8 |             18 |             28 | col_str_name_4-value-8 | col_str_name_5-value-8 | col_str_name_6-value-8 |
+----------------+----------------+----------------+------------------------+------------------------+------------------------+
|              9 |             19 |             29 | col_str_name_4-value-9 | col_str_name_5-value-9 | col_str_name_6-value-9 |
+----------------+----------------+----------------+------------------------+------------------------+------------------------+)";
}

TEST(FormatTableTests, PrintPretty) {
  auto internalTable = test_utils::getInternalTable(10, 3, 3);
  auto formattedTable = FormatTable::format(internalTable).str();
  LOG(INFO) << formattedTable;
  ASSERT_EQ(formattedTable, output);
}

} // namespace rk::projects::mydb