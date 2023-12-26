//
// Created by Rahul  Kushwaha on 9/6/23.
//

#pragma once
#include "applications/mydb/backend/QueryPlan.h"
#include "applications/mydb/backend/SchemaStore.h"
#include "applications/mydb/client/proto/db.pb.h"

namespace rk::projects::mydb {

using Request = std::variant<client::ScanTableRequest, client::AddRowRequest>;

// QueryParser object that is created per request object.
class QueryParser {
 public:
  explicit QueryParser(Request request,
                       std::shared_ptr<SchemaStore> schemaStore);

  [[nodiscard]] QueryPlan parse() const;

 private:
  Request request_;
  std::shared_ptr<SchemaStore> schemaStore_;
};

}  // namespace rk::projects::mydb
