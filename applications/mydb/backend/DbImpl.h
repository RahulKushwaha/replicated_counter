//
// Created by rahul on 12/25/23.
//

#pragma once

#include "applications/mydb/backend/QueryExecutor.h"
#include "applications/mydb/backend/SchemaStore.h"
#include "applications/mydb/include/Db.h"

namespace rk::projects::mydb {

class DbImpl : public Db {
 public:
  DbImpl(std::shared_ptr<SchemaStore> schemaStore,
         std::shared_ptr<RocksReaderWriter> rocks);

 public:
  coro<google::protobuf::Empty> addDatabase(
      const client::AddDatabaseRequest* request) override;

  coro<google::protobuf::Empty> addTable(
      const client::AddTableRequest* request) override;

  coro<client::AddRowResponse> addRow(
      const client::AddRowRequest* request) override;

  coro<client::UpdateRowResponse> updateRow(
      const client::UpdateRowRequest* request) override;

  coro<client::GetRowResponse> getRow(
      const client::GetRowRequest* request) override;

  coro<client::MultiTableOperationResponse> multiTableOperation(
      const client::MultiTableOperationRequest* request) override;

  coro<client::ScanTableResponse> scanTable(
      const client::ScanTableRequest* request) override;

  coro<client::PrepareTransactionResponse> prepareTransaction(
      const client::PrepareTransactionRequest* request) override;
  coro<client::CommitTransactionResponse> commitTransaction(
      const client::CommitTransactionRequest* request) override;
  coro<client::AbortTransactionResponse> abortTransaction(
      const client::AbortTransactionRequest* request) override;

 private:
  Condition addConditionToCheckWriteLock(Condition condition);

 private:
  std::shared_ptr<SchemaStore> schemaStore_;
  std::shared_ptr<QueryExecutor> queryExecutor_;
  TableSchemaType::DbIdType dbIds_;
  TableSchemaType::TableIdType tableIds_;
};

}  // namespace rk::projects::mydb