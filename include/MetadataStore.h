//
// Created by Rahul  Kushwaha on 12/29/22.
//

#ifndef LOGSTORAGE_METADATASTORE_H
#define LOGSTORAGE_METADATASTORE_H

#include "Common.h"
#include "NanoLog.h"
#include "../MetadataConfig.pb.h"

namespace rk::project::counter {
using VersionId = std::int64_t;

struct OptimisticConcurrencyException: public std::exception {
  const char *what() const noexcept override {
    return "C++ Exception";
  }
};


class MetadataStore {
 public:
  virtual MetadataConfig getConfig(VersionId versionId) = 0;

  virtual void
  appendRange(VersionId versionId,
              MetadataConfig newMetadataConfig) = 0;
};

}

#endif //LOGSTORAGE_METADATASTORE_H
