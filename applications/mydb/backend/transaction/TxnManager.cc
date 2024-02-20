//
// Created by Rahul  Kushwaha on 1/16/24.
//

#include "applications/mydb/backend/transaction/TxnManager.h"

namespace rk::projects::mydb::transaction {

TxnManager::TxnManager(std::shared_ptr<TableSchema> schema,
                       std::shared_ptr<Db> db)
    : columnNames_{}, schema_{std::move(schema)}, db_{std::move(db)} {}

folly::coro::Task<std::optional<Transaction>> TxnManager::get(
    std::string transactionId) {
  client::GetRowRequest getTxnRequest{};
  getTxnRequest.mutable_database_name()->set_name(
      schema_->rawTable().db().name());
  getTxnRequest.mutable_table_name()->set_name(schema_->rawTable().name());

  client::ColumnValue columnValue{};
  columnValue.mutable_name()->set_name(columnNames_.id);
  columnValue.mutable_value()->set_value(transactionId);

  getTxnRequest.mutable_primary_key_values()
      ->Add()
      ->mutable_column_values()
      ->Add(std::move(columnValue));

  auto response = db_->getRow(&getTxnRequest);
  co_return Transformer::from(response);
}

folly::coro::Task<bool> TxnManager::create(Transaction transaction) {
  auto addRowRequest = Transformer::toAddRowRequest(transaction);
  auto response = db_->addRow(&addRowRequest);

  co_return true;
}

folly::coro::Task<bool> TxnManager::update(Transaction transaction) {
  auto updateRowRequest = Transformer::toUpdateRowRequest(transaction);
  auto response = db_->updateRow(&updateRowRequest);

  co_return true;
}

folly::coro::Task<bool> TxnManager::del(std::string transactionId) {
  co_return false;
}

}  // namespace rk::projects::mydb::transaction