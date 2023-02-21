//
// Created by Rahul  Kushwaha on 1/2/23.
//

#ifndef LOGSTORAGE_NANOLOGSTOREIMPL_H
#define LOGSTORAGE_NANOLOGSTOREIMPL_H
#include "../include/NanoLogStore.h"
#include <unordered_map>

namespace rk::projects::durable_log {

class NanoLogStoreImpl: public NanoLogStore {
 public:
  explicit NanoLogStoreImpl() = default;
  std::shared_ptr<NanoLog> getNanoLog(VersionId versionId) override;
  void add(VersionId versionId, std::shared_ptr<NanoLog> nanoLog) override;

  ~NanoLogStoreImpl() override = default;
 private:
  std::unordered_map<VersionId, std::shared_ptr<NanoLog>> store_;
};

}

#endif //LOGSTORAGE_NANOLOGSTOREIMPL_H
