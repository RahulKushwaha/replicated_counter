//
// Created by Rahul  Kushwaha on 1/2/23.
//
#pragma once

#include "NanoLog.h"
#include "MetadataStore.h"

namespace rk::projects::durable_log {

class NanoLogStore {
 public:
  virtual std::shared_ptr<NanoLog> getNanoLog(VersionId versionId) = 0;
  virtual void add(VersionId versionId, std::shared_ptr<NanoLog> nanoLog) = 0;
  virtual ~NanoLogStore() = default;
};

}
