//
// Created by Rahul  Kushwaha on 4/9/23.
//

#pragma once
#include "QueryOptions.h"
#include "applications/mydb/backend/TableRow.h"
#include "rocksdb/db.h"

namespace rk::projects::mydb {

class RocksReaderWriter {
public:
  explicit RocksReaderWriter(rocksdb::DB *db);

  bool write(std::vector<RawTableRow> rows);

  std::vector<RawTableRow> read(std::vector<RawTableRow::Key> keys);

  bool del(std::vector<RawTableRow::Key> keys);

  std::vector<RawTableRow> scan(ScanOptions scanOptions);

  std::unique_ptr<rocksdb::ManagedSnapshot> createSnapshot();

private:
  rocksdb::DB *rocks_;
};

} // namespace rk::projects::mydb
