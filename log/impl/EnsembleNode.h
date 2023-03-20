//
// Created by Rahul  Kushwaha on 2/26/23.
//

#pragma once

#include <memory>
#include "log/server/SequencerServer.h"
#include "log/server/ReplicaServer.h"
#include "log/include/Replica.h"
#include "log/include/Registry.h"
#include "log/include/FailureDetector.h"

namespace rk::projects::durable_log {

template<class APP>
struct EnsembleNode {
  std::shared_ptr<APP> app;
  std::shared_ptr<server::SequencerServer> sequencer;
  std::shared_ptr<server::ReplicaServer> replica;
  std::vector<std::shared_ptr<Replica>> replicaSet;
  std::shared_ptr<Registry> registry;
  std::shared_ptr<FailureDetector> failureDetector;

  ~EnsembleNode() {
    LOG(INFO) << "Destructor Called";
  }
};

}