//
// Created by Rahul  Kushwaha on 1/2/23.
//

#pragma once

#include "log/include/NanoLogStore.h"
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