//
// Created by Rahul  Kushwaha on 2/22/23.
//
#pragma once

#include "Replica.h"
#include "Sequencer.h"

namespace rk::projects::durable_log {

class Registry {
 public:

  virtual void
  registerReplica(std::string id, std::shared_ptr<Replica> replica) = 0;
  virtual void
  registerSequencer(std::string id, std::shared_ptr<Sequencer> sequencer) = 0;
  virtual void registerReplica(std::shared_ptr<Replica> replica) = 0;
  virtual void registerSequencer(std::shared_ptr<Sequencer> sequencer) = 0;

  virtual std::shared_ptr<Replica> replica(std::string replicaId) = 0;
  virtual std::shared_ptr<Sequencer> sequencer(std::string sequencerId) = 0;

  virtual ~Registry() = default;
};

}
