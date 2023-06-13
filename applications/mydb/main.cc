//
// Created by Rahul  Kushwaha on 4/9/23.
//
#include <arrow/array.h>
#include <arrow/builder.h>
#include <arrow/chunked_array.h>
#include <arrow/table.h>
#include <iostream>

auto getIntArrays(std::int32_t size = 10) {
  arrow::Int64Builder builder;
  arrow::Status status;
  for (int i = 0; i < 10; i++) {
    if (i % 2 == 0) {
      status = builder.Append(i);
    } else {
      status = builder.AppendNull();
    }

    if (!status.ok()) {
      std::cout << "failed" << status.ToString();
    }
  }

  auto result = builder.Finish();
  return result;
}

auto makeChunkedArray(std::int32_t size = 10) {}

int main(int argc, char **argv) {
  std::cout << "Hello World" << std::endl;
  auto v1 = getIntArrays(10).ValueOrDie();
  auto v2 = getIntArrays(9).ValueOrDie();

  auto schema = arrow::schema({
      arrow::field("v1", arrow::int64()),
      arrow::field("v2", arrow::int64()),
  });

  auto recordBatch = arrow::RecordBatch::Make(schema, 2, {v1, v2});
  auto table = arrow::Table::FromRecordBatches({recordBatch}).ValueOrDie();
  std::cout << table->ToString();
}
