//
// Created by Rahul  Kushwaha on 12/29/22.
//

#include "InMemoryMetadataStore.h"

namespace rk::project::counter {

InMemoryMetadataStore::InMemoryMetadataStore() {
  rangeLookup_[beginSentinel] = std::shared_ptr<NanoLog>();
}

std::shared_ptr<NanoLog> InMemoryMetadataStore::getNanoLog(LogId logId) {
  auto itr = rangeLookup_.upper_bound({logId, logId});
  if (itr->first == beginSentinel) {
    return {};
  }

  itr--;
  return itr->second;
}

void InMemoryMetadataStore::appendRange(LogId startIndex,
                                        std::shared_ptr<NanoLog> nanoLog) {

}

}