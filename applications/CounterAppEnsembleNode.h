//
// Created by Rahul  Kushwaha on 2/26/23.
//
#pragma once

#include <string>
#include "Ensemble.h"
#include "counter/CounterApp.h"
#include "../log/impl/RemoteReplica.h"
#include "../log/impl/RemoteSequencer.h"
#include "../log/impl/VirtualLogFactory.h"
#include "../log/server/ReplicaServer.h"
#include "../log/server/SequencerServer.h"
#include "../log/impl/VirtualLogImpl.h"
#include "../log/utils/GrpcServerFactory.h"
#include <folly/executors/ThreadPoolExecutor.h>

#include <grpcpp/security/credentials.h>
#include <grpcpp/create_channel.h>

namespace rk::projects::counter_app {

using CounterAppEnsembleNode = EnsembleNode<CounterApp>;
using CounterAppEnsemble = Ensemble<CounterApp, 5>;

struct EnsembleNodeConfig {
  std::int32_t sequencerPort;
  std::int32_t replicaPort;
  std::int32_t metadataStorePort;
};

EnsembleNodeConfig ensembleNode1{10'000, 10'001, 10'002};
EnsembleNodeConfig ensembleNode2{20'000, 20'001, 20'002};
EnsembleNodeConfig ensembleNode3{30'000, 30'001, 30'002};
EnsembleNodeConfig ensembleNode4{40'000, 40'001, 40'002};
EnsembleNodeConfig ensembleNode5{50'000, 50'001, 50'002};

std::array<EnsembleNodeConfig, 5>
    configs
    {ensembleNode1, ensembleNode2, ensembleNode3, ensembleNode4, ensembleNode5};

CounterAppEnsemble makeCounterAppEnsemble(std::string appName,
                                          folly::Executor *executor) {

  CounterAppEnsemble counterAppEnsemble;
  std::shared_ptr<MetadataStore> metadataStore = makeMetadataStore();
  MetadataConfig metadataConfig;
  metadataConfig.set_version_id(1);
  metadataConfig.set_previous_version_id(0);
  metadataConfig.set_start_index(1);
  metadataConfig.set_end_index(1000);

  metadataStore->compareAndAppendRange(0, metadataConfig);

  std::shared_ptr<Registry> registry = makeRegistry();

  // TODO(rahul): use cpp fmt.
  std::string serverAddressFormat{"localhost:"};

  std::int32_t index = 0;
  for (auto &config: configs) {
    std::string replicaAddress =
        serverAddressFormat + std::to_string(config.replicaPort);

    auto replicaClient = std::make_shared<client::ReplicaClient>(
        grpc::CreateChannel(replicaAddress, grpc::InsecureChannelCredentials())
    );

    std::shared_ptr<NanoLogStore> nanoLogStore = makeNanoLogStore();

    std::shared_ptr<Replica>
        remoteReplica = std::make_shared<RemoteReplica>(replicaClient);

    std::shared_ptr<Replica>
        localReplica =
        makeReplica(replicaAddress, "", nanoLogStore, metadataStore);

    auto localReplicaServer =
        std::make_shared<server::ReplicaServer>(localReplica);

    durable_log::server::runRPCServer(replicaAddress,
                                      localReplicaServer.get(),
                                      executor,
                                      "Replica")
        .get();

    auto &ensembleNode = counterAppEnsemble.nodes_[index];
    ensembleNode.replica = localReplicaServer;

    // Add current remote replica to other nodes replica set.
    for (std::size_t otherReplica = 0; otherReplica < configs.size();
         otherReplica++) {
      if (index != otherReplica) {
        counterAppEnsemble.nodes_[otherReplica].replicaSet
            .push_back(remoteReplica);
      }
    }
    ensembleNode.replicaSet.push_back(localReplica);

    ensembleNode.registry = registry;

    index++;
  }

  index = 0;
  for (auto &config: configs) {
    auto &ensembleNode = counterAppEnsemble.nodes_[index];
    // All the replica set for each node should be populated by now.
    CHECK(ensembleNode.replicaSet.size() == 5);

    std::string sequencerAddress =
        serverAddressFormat + std::to_string(config.sequencerPort);
    auto sequencerClient = std::make_shared<client::SequencerClient>(
        grpc::CreateChannel(sequencerAddress,
                            grpc::InsecureChannelCredentials())
    );

    LOG(INFO) << "Sequencer: " << sequencerAddress;
    for (const auto &replica: ensembleNode.replicaSet) {
      LOG(INFO) << "  has Replica: " << replica->getId();
    }

    auto remoteSequencer = std::make_shared<RemoteSequencer>(sequencerClient);

    std::shared_ptr<Sequencer>
        sequencer = makeSequencer(ensembleNode.replicaSet);

    std::shared_ptr<server::SequencerServer> localSequencerServer =
        std::make_shared<server::SequencerServer>(sequencer);

    ensembleNode.sequencer = localSequencerServer;

    durable_log::server::runRPCServer(sequencerAddress,
                                      localSequencerServer.get(),
                                      executor,
                                      "Sequencer")
        .get();

    std::shared_ptr<VirtualLog>
        log = std::make_shared<VirtualLogImpl>("id",
                                               appName,
                                               sequencer,
                                               ensembleNode.replicaSet,
                                               metadataStore,
                                               metadataConfig.version_id());

    auto counterApp = std::make_shared<CounterApp>(log);
    ensembleNode.app = counterApp;

    index++;
  }


  return counterAppEnsemble;
}

}
