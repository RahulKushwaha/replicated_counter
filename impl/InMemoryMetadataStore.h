//
// Created by Rahul  Kushwaha on 12/29/22.
//

#ifndef LOGSTORAGE_INMEMORYMETADATASTORE_H
#define LOGSTORAGE_INMEMORYMETADATASTORE_H

#include "../include/MetadataStore.h"

#include <map>

namespace rk::project::counter {

class InMemoryMetadataStore: public MetadataStore {
 public:
  explicit InMemoryMetadataStore();

  std::shared_ptr<NanoLog> getNanoLog(LogId logId) override;

  void appendRange(LogId startIndex, std::shared_ptr<NanoLog> nanoLog) override;

 private:
  struct LogRange {
    LogId startIndex_;
    LogId endIndex_;

    bool operator<(const LogRange &other) const {
      return startIndex_ < other.startIndex_;
    }

    bool operator==(const LogRange &other) const {
      return startIndex_ == other.startIndex_ && endIndex_ == other.endIndex_;
    }
  };

  static constexpr LogRange
      beginSentinel{LowestNonExistingLogId, LowestNonExistingLogId};
  static constexpr LogRange
      endSentinel{HighestNonExistingLogId, HighestNonExistingLogId};

  std::map<LogRange, std::shared_ptr<NanoLog>> rangeLookup_;
};

}
#endif //LOGSTORAGE_INMEMORYMETADATASTORE_H
