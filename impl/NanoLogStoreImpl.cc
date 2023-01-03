//
// Created by Rahul  Kushwaha on 1/2/23.
//

#include "NanoLogStoreImpl.h"
namespace rk::project::counter {

std::shared_ptr<NanoLog>
NanoLogStoreImpl::getNanoLog(VersionId versionId) {
  if (auto itr = store_.find(versionId); itr != store_.end()) {
    return itr->second;
  }

  return {};
}

void
NanoLogStoreImpl::add(VersionId versionId, std::shared_ptr<NanoLog> nanoLog) {
  store_.emplace(versionId, nanoLog);
}

}
