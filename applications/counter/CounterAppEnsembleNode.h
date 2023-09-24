//
// Created by Rahul  Kushwaha on 2/26/23.
//
#pragma once

#include "CounterApp.h"
#include "applications/counter/CounterHealthCheck.h"
#include "folly/executors/CPUThreadPoolExecutor.h"
#include "folly/executors/ThreadPoolExecutor.h"
#include "log/impl/Ensemble.h"
#include "log/impl/EnsembleConfig.h"
#include "log/impl/FailureDetectorImpl.h"
#include "log/impl/NullSequencer.h"
#include "log/impl/RemoteReplica.h"
#include "log/impl/RemoteSequencer.h"
#include "log/impl/VirtualLogFactory.h"
#include "log/impl/VirtualLogImpl.h"
#include "log/server/ReplicaServer.h"
#include "log/server/SequencerServer.h"
#include "log/utils/GrpcServerFactory.h"
#include "log/utils/UuidGenerator.h"
#include <string>

#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

namespace rk::projects::counter_app {

using CounterAppEnsembleNode = EnsembleNode<CounterApp>;
using CounterAppEnsemble = Ensemble<CounterApp, 5>;

EnsembleNodeConfig ensembleNode1{
    .sequencerId = "SEQUENCER_" + utils::UuidGenerator::instance().generate(),
    .sequencerPort = 10'000,

    .replicaId = "REPLICA_" + utils::UuidGenerator::instance().generate(),
    .replicaPort = 10'001,
    .metadataStorePort = 10'002};

EnsembleNodeConfig ensembleNode2{
    .sequencerId = "SEQUENCER_" + utils::UuidGenerator::instance().generate(),
    .sequencerPort = 20'000,

    .replicaId = "REPLICA_" + utils::UuidGenerator::instance().generate(),
    .replicaPort = 20'001,

    .metadataStorePort = 20'002};

EnsembleNodeConfig ensembleNode3{
    .sequencerId = "SEQUENCER_" + utils::UuidGenerator::instance().generate(),
    .sequencerPort = 30'000,

    .replicaId = "REPLICA_" + utils::UuidGenerator::instance().generate(),
    .replicaPort = 30'001,

    .metadataStorePort = 30'002};

EnsembleNodeConfig ensembleNode4{
    .sequencerId = "SEQUENCER_" + utils::UuidGenerator::instance().generate(),
    .sequencerPort = 40'000,

    .replicaId = "REPLICA_" + utils::UuidGenerator::instance().generate(),
    .replicaPort = 40'001,

    .metadataStorePort = 40'002};

EnsembleNodeConfig ensembleNode5{
    .sequencerId = "SEQUENCER_" + utils::UuidGenerator::instance().generate(),
    .sequencerPort = 50'000,

    .replicaId = "REPLICA_" + utils::UuidGenerator::instance().generate(),
    .replicaPort = 50'001,

    .metadataStorePort = 50'002};

std::array<EnsembleNodeConfig, 5> configs{
    ensembleNode1, ensembleNode2, ensembleNode3, ensembleNode4, ensembleNode5};

CounterAppEnsemble makeCounterAppEnsemble(std::string appName,
                                          folly::Executor *executor) {

  CounterAppEnsemble counterAppEnsemble;
  std::shared_ptr<MetadataStore> metadataStore = makeMetadataStore();
  MetadataConfig metadataConfig;
  metadataConfig.set_version_id(1);
  metadataConfig.set_previous_version_id(0);
  metadataConfig.set_start_index(1);
  metadataConfig.set_end_index(1000);

  metadataStore->compareAndAppendRange(metadataConfig).semi().get();

  std::shared_ptr<Registry> registry = makeRegistry();

  // TODO(rahul): use cpp fmt.
  std::string serverAddressFormat{"localhost:"};

  std::int32_t index = 0;
  for (auto &config: configs) {
    std::string replicaAddress =
        serverAddressFormat + std::to_string(config.replicaPort);

    auto replicaClient =
        std::make_shared<client::ReplicaClient>(grpc::CreateChannel(
            replicaAddress, grpc::InsecureChannelCredentials()));

    std::shared_ptr<NanoLogStore> nanoLogStore = makeNanoLogStore();

    std::shared_ptr<Replica> remoteReplica =
        std::make_shared<RemoteReplica>(replicaClient);

    std::shared_ptr<Replica> localReplica =
        makeReplica(config.replicaId, "", nanoLogStore, metadataStore);

    auto localReplicaServer =
        std::make_shared<rk::projects::durable_log::server::ReplicaServer>(
            localReplica);

    durable_log::server::runRPCServer(
        replicaAddress, {localReplicaServer.get()}, executor, "Replica")
        .get();

    registry->registerReplica(remoteReplica);

    auto &ensembleNode = counterAppEnsemble.nodes_[index];
    ensembleNode.replica = localReplicaServer;

    // Add current remote replica to other nodes replica set.
    for (std::size_t otherReplica = 0; otherReplica < configs.size();
         otherReplica++) {
      if (index != otherReplica) {
        counterAppEnsemble.nodes_[otherReplica].replicaSet.push_back(
            remoteReplica);
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
    auto sequencerClient =
        std::make_shared<client::SequencerClient>(grpc::CreateChannel(
            sequencerAddress, grpc::InsecureChannelCredentials()));

    LOG(INFO) << "Sequencer: " << sequencerAddress;
    for (const auto &replica: ensembleNode.replicaSet) {
      LOG(INFO) << "  has Replica: " << replica->getId();
    }

    auto remoteSequencer = std::make_shared<RemoteSequencer>(sequencerClient);

    std::shared_ptr<Sequencer> sequencer =
        makeSequencer(config.sequencerId, ensembleNode.replicaSet);

    auto localSequencerServer =
        std::make_shared<rk::projects::durable_log::server::SequencerServer>(
            sequencer);

    ensembleNode.sequencer = localSequencerServer;

    durable_log::server::runRPCServer(
        sequencerAddress, {localSequencerServer.get()}, executor, "Sequencer")
        .get();

    registry->registerSequencer(remoteSequencer);

    std::shared_ptr<VirtualLog> log = std::make_shared<VirtualLogImpl>(
        "VIRTUAL_LOG_" + utils::UuidGenerator::instance().generate(), appName,
        std::make_shared<NullSequencer>(), ensembleNode.replicaSet,
        metadataStore, metadataConfig.version_id(), registry);

    auto counterApp = std::make_shared<CounterApp>(nullptr, nullptr);
    ensembleNode.app = counterApp;
    if (true) {
      std::shared_ptr<folly::Executor> failureDetectorPool =
          std::make_shared<folly::CPUThreadPoolExecutor>(2);

      std::shared_ptr<HealthCheck> healthCheck =
          std::make_shared<CounterHealthCheck>(ensembleNode.app);

      std::vector<EnsembleNodeConfig> replicaConfigs{configs.begin(),
                                                     configs.end()};

      std::shared_ptr<FailureDetector> failureDetector =
          std::make_shared<FailureDetectorImpl>(
              healthCheck, log, failureDetectorPool,
              rk::projects::server::ServerConfig{});
      ensembleNode.failureDetector = failureDetector;
    }

    index++;
  }

  return counterAppEnsemble;
}

} // namespace rk::projects::counter_app
