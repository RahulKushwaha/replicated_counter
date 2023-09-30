//
// Created by Rahul  Kushwaha on 9/12/23.
//

#pragma once

#include "applications/mydb/backend/TableRow.h"
#include "tabulate/table.hpp"

#include <arrow/array.h>
#include <arrow/table.h>

namespace rk::projects::mydb {

class FormatTable {
 public:
  static tabulate::Table format(const InternalTable& internalTable) {
    auto& arrowTable = *internalTable.table;

    tabulate::Table tabulateTable;

    tabulate::Table::Row_t header;
    for (const auto& col : internalTable.schema->rawTable().columns()) {
      header.emplace_back(col.name());
    }
    tabulateTable.add_row(header);

    for (std::int32_t rowIdx = 0; rowIdx < arrowTable.num_rows(); rowIdx++) {
      auto totalChunks = arrowTable.columns().front()->num_chunks();
      for (std::int32_t chunkIdx = 0; chunkIdx < totalChunks; chunkIdx++) {

        tabulate::Table::Row_t tabulateRow;

        for (const auto& col : internalTable.schema->rawTable().columns()) {
          auto arrowCol =
              arrowTable.GetColumnByName(col.name())->chunk(chunkIdx);

          if (col.column_type() ==
              mydb::Column_COLUMN_TYPE::Column_COLUMN_TYPE_INT64) {
            auto chunk = std::static_pointer_cast<arrow::Int64Array>(arrowCol);
            auto val = chunk->Value(rowIdx);

            tabulateRow.emplace_back(std::to_string(val));
          } else if (col.column_type() ==
                     mydb::Column_COLUMN_TYPE::Column_COLUMN_TYPE_STRING) {
            auto chunk = std::static_pointer_cast<arrow::StringArray>(arrowCol);
            auto val = chunk->Value(rowIdx);

            tabulateRow.emplace_back(std::string{val});
          }
        }

        tabulateTable.add_row(tabulateRow);
      }
    }

    for (auto col = 0; col < internalTable.schema->rawTable().columns().size();
         col++) {
      tabulateTable.column(col).format().font_align(tabulate::FontAlign::right);

      tabulateTable[0][col]
          .format()
          .font_color(tabulate::Color::yellow)
          .font_align(tabulate::FontAlign::center)
          .font_style({tabulate::FontStyle::bold});
    }

    return tabulateTable;
  }
};

}  // namespace rk::projects::mydb
