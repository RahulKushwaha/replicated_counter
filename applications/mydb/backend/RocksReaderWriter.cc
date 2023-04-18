//
// Created by Rahul  Kushwaha on 4/9/23.
//

#include "RocksReaderWriter.h"
#include "rocksdb/utilities/write_batch_with_index.h"

namespace rk::projects::mydb {

RocksReaderWriter::RocksReaderWriter(rocksdb::DB *db) : rocks_{db} {}

bool RocksReaderWriter::write(std::vector<RawTableRow> rows) {
  rocksdb::WriteBatchWithIndex batch{};
  for (auto &row: rows) {
    for (auto &[k, v]: row.keyValues) {
      batch.Put(k, v);
    }
  }

  rocksdb::Status
      status = rocks_->Write(rocksdb::WriteOptions{}, batch.GetWriteBatch());

  return status.ok();
}

bool RocksReaderWriter::del(std::vector<RawTableRow::Key> keys) {
  rocksdb::WriteBatchWithIndex batch{};
  for (auto &key: keys) {
    batch.Delete(key);
  }

  rocksdb::Status
      status = rocks_->Write(rocksdb::WriteOptions{}, batch.GetWriteBatch());

  return status.ok();
}

std::vector<RawTableRow>
RocksReaderWriter::read(std::vector<RawTableRow::Key> keys) {
  std::vector<RawTableRow> rows;
  for (auto &key: keys) {
    RawTableRow row;

    auto itr = rocks_->NewIterator(rocksdb::ReadOptions{});
    itr->Seek(key);
    while (itr->Valid()) {
      if (itr->key().starts_with(key)) {
        row.keyValues.emplace_back(itr->key().ToString(),
                                   itr->value().ToString());
      } else {
        break;
      }

      itr->Next();
    }

    rows.emplace_back(std::move(row));
  }

  return rows;
}

std::vector<RawTableRow>
RocksReaderWriter::scan(std::string prefix, ScanDirection direction) {
  std::vector<RawTableRow> rows;

  auto itr = rocks_->NewIterator(rocksdb::ReadOptions{});
  itr->Seek(prefix);
  std::string prevKey = "SOME WEIRD STRING";

  RawTableRow row;
  while (itr->Valid()) {
    LOG(INFO) << itr->key().ToString();
    if (itr->key().starts_with(prevKey)) {

    } else if (itr->key().starts_with(prefix)) {
      rows.emplace_back(std::move(row));
      row = RawTableRow{};
      prevKey = itr->key().ToString();
    } else {
      break;
    }

    row.keyValues.emplace_back(itr->key().ToString(),
                               itr->value().ToString());

    itr->Next();
  }

  rows.emplace_back(std::move(row));

  return rows;
}

std::unique_ptr<rocksdb::ManagedSnapshot> RocksReaderWriter::createSnapshot() {
  return std::make_unique<rocksdb::ManagedSnapshot>(rocks_);
}

}