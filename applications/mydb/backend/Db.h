//
// Created by rahul on 12/25/23.
//

#pragma once

#include "applications/mydb/backend/QueryExecutor.h"
#include "applications/mydb/backend/SchemaStore.h"
#include "applications/mydb/client/proto/db.grpc.pb.h"
#include "google/protobuf/empty.pb.h"

namespace rk::projects::mydb {

class Db {
 public:
  Db(std::shared_ptr<SchemaStore> schemaStore,
     std::shared_ptr<RocksReaderWriter> rocks);

 public:
  google::protobuf::Empty addDatabase(
      const client::AddDatabaseRequest* request);

  google::protobuf::Empty addTable(const client::AddTableRequest* request);

  client::AddRowResponse addRow(const client::AddRowRequest* request);

  client::UpdateRowResponse updateRow(const client::UpdateRowRequest* request);

  client::TableRows scanDatabase(const client::ScanTableRequest* request);

 private:
  client::Condition addConditionToCheckWriteLock(client::Condition condition);

 private:
  std::shared_ptr<SchemaStore> schemaStore_;
  std::shared_ptr<QueryExecutor> queryExecutor_;
  TableSchemaType::DbIdType dbIds_;
  TableSchemaType::TableIdType tableIds_;
};

}  // namespace rk::projects::mydb