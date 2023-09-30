//
// Created by Rahul  Kushwaha on 9/6/23.
//

#pragma once
#include "applications/mydb/backend/QueryPlan.h"
#include "applications/mydb/backend/SchemaStore.h"
#include "applications/mydb/client/proto/db.pb.h"

namespace rk::projects::mydb {

// QueryParser object that is created per request object.
class QueryParser {
 public:
  explicit QueryParser(client::ScanTableRequest scanTableRequest,
                       std::shared_ptr<SchemaStore> schemaStore);

  QueryPlan parse();

 private:
  client::ScanTableRequest scanTableRequest_;
  std::shared_ptr<SchemaStore> schemaStore_;
};

}  // namespace rk::projects::mydb
