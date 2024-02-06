//
// Created by Rahul  Kushwaha on 1/16/24.
//

#pragma once
#include "applications/mydb/backend/Common.h"
#include "applications/mydb/backend/transaction/proto/txn.pb.h"
#include "applications/mydb/include/Db.h"
#include "folly/experimental/coro/Task.h"

namespace rk::projects::mydb::transaction {

class TxnManager {
 public:
  explicit TxnManager(std::shared_ptr<TableSchema> schema,
                      std::shared_ptr<Db> db);

  folly::coro::Task<std::optional<Transaction>> get(std::string transactionId);
  folly::coro::Task<bool> create(Transaction transaction);
  folly::coro::Task<bool> update(Transaction transaction);
  folly::coro::Task<bool> del(std::string transactionId);

 public:
  struct Transformer {
    static client::AddRowRequest toAddRowRequest(const Transaction& txn);
    static client::UpdateRowRequest toUpdateRowRequest(const Transaction& txn);
    static std::optional<Transaction> from(
        const client::GetRowResponse& response);
    static Transaction from(const std::string& transactionId);
  };

 private:
  struct ColumnNames {
    std::string id = "id";
    std::string data = "data";
    std::string debugString = "debug_string";
    std::string version = "version";
  };

 private:
  ColumnNames columnNames_;
  std::shared_ptr<TableSchema> schema_;
  std::shared_ptr<Db> db_;
};

}  // namespace rk::projects::mydb::transaction