//
// Created by Rahul  Kushwaha on 2/5/24.
//

#pragma once

#include "applications/mydb/client/proto/db.pb.h"
#include "folly/experimental/coro/Task.h"
#include "google/protobuf/empty.pb.h"

namespace rk::projects::mydb {

template <typename T>
using coro = folly::coro::Task<T>;

class Db {
 public:
  virtual coro<google::protobuf::Empty> addDatabase(
      const client::AddDatabaseRequest* request) = 0;

  virtual coro<google::protobuf::Empty> addTable(
      const client::AddTableRequest* request) = 0;

  virtual coro<client::AddRowResponse> addRow(
      const client::AddRowRequest* request) = 0;

  virtual coro<client::UpdateRowResponse> updateRow(
      const client::UpdateRowRequest* request) = 0;

  virtual coro<client::GetRowResponse> getRow(
      const client::GetRowRequest* request) = 0;

  virtual coro<client::MultiTableOperationResponse> multiTableOperation(
      const client::MultiTableOperationRequest* request) = 0;

  virtual coro<client::ScanTableResponse> scanTable(
      const client::ScanTableRequest* request) = 0;

  virtual coro<client::PrepareTransactionResponse> prepareTransaction(
      const client::PrepareTransactionRequest* request) = 0;
  virtual coro<client::CommitTransactionResponse> commitTransaction(
      const client::CommitTransactionRequest* request) = 0;
  virtual coro<client::AbortTransactionResponse> abortTransaction(
      const client::AbortTransactionRequest* request) = 0;
};

}  // namespace rk::projects::mydb
