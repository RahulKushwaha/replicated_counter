//
// Created by Rahul  Kushwaha on 12/29/22.
//

#ifndef LOGSTORAGE_METADATASTORE_H
#define LOGSTORAGE_METADATASTORE_H

#include "Common.h"
#include "NanoLog.h"

namespace rk::project::counter {

class MetadataStore {
 public:
  virtual std::shared_ptr<NanoLog> getNanoLog(LogId logId) = 0;

  virtual void
  appendRange(LogId startIndex, std::shared_ptr<NanoLog> nanoLog) = 0;
};

}

#endif //LOGSTORAGE_METADATASTORE_H
