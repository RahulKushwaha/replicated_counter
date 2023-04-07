//
// Created by Rahul  Kushwaha on 4/7/23.
//
#pragma once

#include "TableRow.h"
#include "AddTableRowRequest.h"
#include "folly/Conv.h"
#include "KeySerializer.h"

namespace rk::projects::mydb {

constexpr std::string_view EMPTY_ROW_VALUE;

class RowSerializer {
 public:
  static RawTableRow serialize(AddTableRowRequest addTableRowRequest) {
    RawTableRow rawTableRow{};
    {
      // add to the primary index
      std::vector<std::uint32_t> primaryIndexColumnIds;
      for (auto columnId:
          addTableRowRequest.table->rawTable().primary_key_index().column_ids()) {
        primaryIndexColumnIds.emplace_back(columnId);
      }

      auto rowValues = getRowValues(primaryIndexColumnIds, addTableRowRequest);
      std::string
          primaryKey =
          prefix::primaryKey(addTableRowRequest.table->rawTable(), rowValues);

      rawTableRow.keyValues.emplace_back(primaryKey, EMPTY_ROW_VALUE);

      LOG(INFO) << "finished primary key value";

      // add column values
      for (const auto &col: addTableRowRequest.addRowRequest.column_values()) {
        auto colKey = prefix::columnKey(primaryKey,
                                        addTableRowRequest.table->getColumnId(
                                            col.name().name()));

        rawTableRow.keyValues.emplace_back(std::move(colKey),
                                           toString(toColumnValue(col.value())));
      }
    }

    // add to secondary index/es
    for (const auto
          &index: addTableRowRequest.table->rawTable().secondary_index()) {
      std::vector<std::uint32_t> secondaryIndexColumnIds;
      for (const auto columnId: index.column_ids()) {
        secondaryIndexColumnIds.emplace_back(columnId);
      }

      auto
          rowValues = getRowValues(secondaryIndexColumnIds, addTableRowRequest);

      auto secondaryIndexKey =
          prefix::secondaryIndexKey(addTableRowRequest.table->rawTable(),
                                    index.id(),
                                    rowValues);

      rawTableRow.keyValues.emplace_back(std::move(secondaryIndexKey),
                                         EMPTY_ROW_VALUE);
    }

    return rawTableRow;
  }

  static std::vector<ColumnValue>
  getRowValues(const std::vector<std::uint32_t> &columnIds,
               const internal::AddTableRowRequest &addRowRequest) {
    std::vector<ColumnValue> rowValues;
    for (auto columnId: columnIds) {
      const auto &columnValue =
          addRowRequest.columnNameToValueLookup.at(columnId);

      if (columnValue.has_value()) {
        rowValues.emplace_back(toColumnValue(columnValue.value()));
      }
    }

    LOG(INFO) << rowValues.size();
    return rowValues;
  }

  static ColumnValue toColumnValue(const google::protobuf::Any &columnValue) {
    if (columnValue.type_url() == "std::string") {
      return columnValue.value();
    } else if (columnValue.GetTypeName() == "std::uint64") {
      return folly::to<std::int64_t>(columnValue.value());
    } else {
      LOG(INFO) << "unknown type " << columnValue.type_url();
      throw std::runtime_error{"unknown type for serialization"};
    }
  }
};

}