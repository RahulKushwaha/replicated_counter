//
// Created by Rahul  Kushwaha on 3/5/23.
//

#pragma once
#include "../include/Registry.h"
#include "../include/Sequencer.h"

#include <unordered_map>

namespace rk::projects::durable_log {

class RegistryImpl : public Registry {
public:
  void registerReplica(std::shared_ptr<Replica> replica) override;
  void registerSequencer(std::shared_ptr<Sequencer> sequencer) override;
  void registerReplica(std::string id,
                       std::shared_ptr<Replica> replica) override;
  void registerSequencer(std::string id,
                         std::shared_ptr<Sequencer> sequencer) override;
  std::shared_ptr<Replica> replica(std::string replicaId) override;
  std::shared_ptr<Sequencer> sequencer(std::string sequencerId) override;

  ~RegistryImpl() override = default;

private:
  std::unordered_map<std::string, std::shared_ptr<Replica>> replicas_;
  std::unordered_map<std::string, std::shared_ptr<Sequencer>> sequencers_;
};

} // namespace rk::projects::durable_log
