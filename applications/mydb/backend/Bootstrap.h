//
// Created by rahul on 12/26/23.
//

#pragma once

#include "applications/mydb/backend/SchemaStore.h"
#include "applications/mydb/backend/proto/db.pb.h"
#include "applications/mydb/format/FormatTable.h"
#include "glog/logging.h"
#include "google/protobuf/text_format.h"

#include <fstream>
#include <sstream>

namespace rk::projects::mydb {

bool bootstrap(std::string metaSchemaDbFileLocation,
               std::string metaSchemaTableFileLocation,
               std::shared_ptr<SchemaStore> schemaStore) {
  internal::Table metaDbSchema;
  internal::Table metaTableSchema;

  {
    std::ifstream file(metaSchemaDbFileLocation);

    if (!file) {
      LOG(INFO) << "file could not be opened";
      return false;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string fileContents = ss.str();

    auto result = google::protobuf::TextFormat::ParseFromString(fileContents,
                                                                &metaDbSchema);

    if (!result) {
      LOG(INFO) << "failed to parse meta_db_schema. quitting";
      return false;
    }
  }

  {
    std::ifstream file(metaSchemaTableFileLocation);

    if (!file) {
      LOG(INFO) << "file could not be opened";
      return false;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string fileContents = ss.str();

    auto result = google::protobuf::TextFormat::ParseFromString(
        fileContents, &metaTableSchema);

    if (!result) {
      LOG(INFO) << "failed to parse meta_table_schema. quitting";
      return false;
    }
  }

  auto result = schemaStore->registerTable(metaDbSchema.db().name(),
                                           metaDbSchema.name(), metaDbSchema);
  if (!result) {
    LOG(INFO) << "failed to register metaDbSchema table. quitting";
    return false;
  }

  result = schemaStore->registerTable(metaTableSchema.db().name(),
                                      metaTableSchema.name(), metaTableSchema);
  if (!result) {
    LOG(INFO) << "failed to register metaTableSchema table. quitting";
    return false;
  }

  return true;
}

}  // namespace rk::projects::mydb