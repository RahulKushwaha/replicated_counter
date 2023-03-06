//
// Created by Rahul  Kushwaha on 3/5/23.
//

#include "RegistryImpl.h"

namespace rk::projects::durable_log {

void RegistryImpl::registerReplica(std::shared_ptr<Replica> replica) {
  std::string replicaId = replica->getId();
  replicas_.emplace(std::move(replicaId), std::move(replica));
}

void RegistryImpl::registerSequencer(std::shared_ptr<Sequencer> sequencer) {
  std::string sequencerId = sequencer->getId();
  sequencers_.emplace(std::move(sequencerId), std::move(sequencer));
}

std::shared_ptr<Replica> RegistryImpl::replica(std::string replicaId) {
  if (auto itr = replicas_.find(replicaId); itr != replicas_.end()) {
    return itr->second;
  }

  return nullptr;
}

std::shared_ptr<Sequencer> RegistryImpl::sequencer(std::string sequencerId) {
  if (auto itr = sequencers_.find(sequencerId); itr != sequencers_.end()) {
    return itr->second;
  }

  return nullptr;
}

}