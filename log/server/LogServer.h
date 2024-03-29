//
// Created by Rahul  Kushwaha on 3/26/23.
//
#pragma once
#include "ReplicaServer.h"
#include "SequencerServer.h"
#include "fmt/format.h"
#include "folly/executors/CPUThreadPoolExecutor.h"
#include "folly/experimental/coro/Task.h"
#include "log/impl/NullSequencer.h"
#include "log/impl/PersistentMetadataStore.h"
#include "log/impl/RegistryImpl.h"
#include "log/impl/RemoteMetadataStore.h"
#include "log/impl/RemoteReplica.h"
#include "log/impl/RemoteSequencer.h"
#include "log/impl/RocksNanoLog.h"
#include "log/impl/VirtualLogFactory.h"
#include "log/impl/VirtualLogImpl.h"
#include "log/include/NanoLogStore.h"
#include "log/proto/Options.pb.h"
#include "log/server/MetadataServer.h"
#include "log/server/proto/ServerConfig.pb.h"
#include "log/utils/GrpcServerFactory.h"
#include "log/utils/UuidGenerator.h"
#include "metrics/MetricsRegistry.h"
#include "prometheus/exposer.h"
#include "wor/paxos/LocalAcceptor.h"
#include "wor/paxos/RegistryImpl.h"
#include "wor/paxos/RemoteAcceptor.h"
#include "wor/paxos/client/AcceptorClient.h"
#include "wor/paxos/server/Acceptor.h"

#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include <utility>

namespace rk::projects::durable_log::server {

using namespace prometheus;

constexpr std::string_view GRPC_SVC_ADDRESS_FORMAT = {"{}:{}"};
constexpr bool USE_PERSISTENT_METADATA_STORE = true;

class LogServer {
 public:
  explicit LogServer(::rk::projects::server::ServerConfig serverConfig,
                     ::rk::projects::durable_log::Options options = {})
      : config_{std::move(serverConfig)},
        options_{std::move(options)},
        mtx_{std::make_unique<std::mutex>()} {}

  folly::coro::Task<void> start() {
    std::lock_guard lockGuard{*mtx_};
    if (state_ != nullptr) {
      co_return;
    }

    State state{.config = config_};
    state.makeMetricExposer();

    state.registry = std::make_shared<RegistryImpl>();
    state.paxosRegistry = std::make_shared<paxos::RegistryImpl>();
    state.threadPoolExecutor =
        std::make_shared<folly::CPUThreadPoolExecutor>(24);
    state.nanoLogStore = std::make_shared<NanoLogStoreImpl>();

    state.makeRemoteMetadataStore();
    state.makeReplicaServer();
    state.makeReplicaSet();
    state.makeAcceptorSet();
    state.makeSequencerServer();
    state.registerSequencers();
    state.makeVirtualLog();

    state_ = std::make_shared<State>(std::move(state));

    // ask the exposer to scrape the registry on incoming HTTP requests
    state_->exposer->RegisterCollectable(
        metrics::MetricsRegistry::instance().registryWeakRef());

    co_return;
  }

  folly::coro::Task<void> stop() {
    std::lock_guard lockGuard{*mtx_};
    if (state_ != nullptr) {
      co_return;
    }

    state_->sequencerGRPCServer->Shutdown();
    state_->replicaGRPCServer->Shutdown();

    state_->threadPoolExecutor->stop();

    co_return;
  }

  std::shared_ptr<VirtualLog> getVirtualLog() { return state_->virtualLog; }

  std::shared_ptr<folly::CPUThreadPoolExecutor> getThreadPool() {
    return state_->threadPoolExecutor;
  }

 private:
  struct State {
    rk::projects::server::ServerConfig config;

    std::shared_ptr<Exposer> exposer;

    std::shared_ptr<Registry> registry;

    std::shared_ptr<paxos::RegistryImpl> paxosRegistry;

    std::shared_ptr<folly::CPUThreadPoolExecutor> threadPoolExecutor;

    std::shared_ptr<NanoLogStore> nanoLogStore;

    std::shared_ptr<MetadataStore> metadataStore;

    std::vector<std::shared_ptr<Replica>> replicaSet;

    std::shared_ptr<Replica> replica;
    std::shared_ptr<ReplicaServer> replicaServer;

    std::shared_ptr<paxos::Acceptor> acceptor;
    std::shared_ptr<paxos::server::AcceptorServer> acceptorServer;

    std::shared_ptr<grpc::Server> replicaGRPCServer;

    std::shared_ptr<Sequencer> sequencer;
    std::shared_ptr<SequencerServer> sequencerServer;
    std::shared_ptr<grpc::Server> sequencerGRPCServer;

    std::shared_ptr<MetadataStore> persistentMetadataStore;

    std::shared_ptr<server::MetadataServer> metadataServer;
    std::shared_ptr<grpc::Server> metadataGRPCServer;

    std::shared_ptr<VirtualLog> virtualLog;

    void makeMetricExposer() {
      const auto address =
          fmt::format(GRPC_SVC_ADDRESS_FORMAT,
                      config.metric_server_config().ip_address().host(),
                      config.metric_server_config().ip_address().port());

      exposer = std::make_shared<Exposer>(address);
    }

    void makeRemoteMetadataStore() {
      if (USE_PERSISTENT_METADATA_STORE) {

        MetadataConfig metadataConfig{};
        metadataConfig.set_start_index(0);
        metadataConfig.set_end_index(1);
        metadataConfig.set_version_id(0);
        metadataConfig.set_previous_version_id(-1);

        for (const auto& replicaServerConfig : config.replica_set()) {
          ReplicaConfig replicaConfig{};
          replicaConfig.set_id(replicaServerConfig.id());
          replicaConfig.mutable_ip_address()->CopyFrom(
              replicaServerConfig.ip_address());
          replicaConfig.set_local(false);

          metadataConfig.mutable_replica_set_config()->Add(
              std::move(replicaConfig));
        }

        metadataStore = std::make_shared<PersistentMetadataStore>(
            std::move(metadataConfig),
            std::make_unique<InMemoryMetadataStore>(), paxosRegistry);
        metadataServer =
            std::make_shared<server::MetadataServer>(metadataStore);

        const auto address =
            fmt::format(GRPC_SVC_ADDRESS_FORMAT,
                        config.metadata_config().ip_address().host(),
                        config.metadata_config().ip_address().port());

        metadataGRPCServer = durable_log::server::runRPCServer(
                                 address, {metadataServer.get()},
                                 threadPoolExecutor.get(), "MetadataServer")
                                 .get();
      } else {
        const auto address =
            fmt::format(GRPC_SVC_ADDRESS_FORMAT,
                        config.metadata_config().ip_address().host(),
                        config.metadata_config().ip_address().port());

        std::shared_ptr<client::MetadataStoreClient> metadataStoreClient =
            std::make_shared<client::MetadataStoreClient>(grpc::CreateChannel(
                address, grpc::InsecureChannelCredentials()));

        metadataStore =
            std::make_shared<RemoteMetadataStore>(metadataStoreClient);
      }
    }

    void makeReplicaServer() {
      const auto address = fmt::format(
          GRPC_SVC_ADDRESS_FORMAT, config.replica_config().ip_address().host(),
          config.replica_config().ip_address().port());
      auto rocksConfig = persistence::RocksDbFactory::RocksDbConfig{
          .path = config.replica_config().data_directory(),
          .createIfMissing = true,
          .manualWALFlush = true,
      };

      replica =
          makeRocksReplica(config.replica_config().id(), "ReplicaName",
                           nanoLogStore, metadataStore, std::move(rocksConfig));

      registry->registerReplica(config.replica_config().id(), replica);

      replicaServer = std::make_shared<server::ReplicaServer>(replica);

      // Replica port is also used for acceptor.
      // TODO: Run all GRPC services on a single port.
      {
        auto paxosRocksConfig = persistence::RocksDbFactory::RocksDbConfig{
            .path = config.paxos_config().data_directory(),
            .createIfMissing = true,
            .manualWALFlush = true,
        };

        auto paxosRocks =
            persistence::RocksDbFactory::provideSharedPtr(paxosRocksConfig);
        auto paxosKVStore = std::make_shared<persistence::RocksKVStoreLite>(
            std::move(paxosRocks));
        acceptor =
            std::make_shared<paxos::LocalAcceptor>("", std::move(paxosKVStore));
        acceptorServer =
            std::make_shared<paxos::server::AcceptorServer>(acceptor);
      }

      replicaGRPCServer =
          durable_log::server::runRPCServer(
              address, {replicaServer.get(), acceptorServer.get()},
              threadPoolExecutor.get(), "Replica")
              .get();

      paxosRegistry->registerAcceptor(config.replica_config().id(), acceptor);
    }

    void makeAcceptorSet() {
      for (const auto& remoteReplicaConfig : config.replica_set()) {
        const auto remoteAcceptorClientAddress = fmt::format(
            GRPC_SVC_ADDRESS_FORMAT, remoteReplicaConfig.ip_address().host(),
            remoteReplicaConfig.ip_address().port());
        auto acceptorClient = std::make_shared<paxos::client::AcceptorClient>(
            grpc::CreateChannel(remoteAcceptorClientAddress,
                                grpc::InsecureChannelCredentials()));

        auto remoteAcceptor =
            std::make_shared<paxos::RemoteAcceptor>(std::move(acceptorClient));

        if (remoteReplicaConfig.id() != replica->getId()) {
          paxosRegistry->registerAcceptor(remoteReplicaConfig.id(),
                                          remoteAcceptor);
        }
      }

      CHECK(replicaSet.size() == config.replica_set().size());
    }

    void makeReplicaSet() {
      for (const auto& remoteReplicaConfig : config.replica_set()) {
        const auto remoteReplicaClientAddress = fmt::format(
            GRPC_SVC_ADDRESS_FORMAT, remoteReplicaConfig.ip_address().host(),
            remoteReplicaConfig.ip_address().port());
        std::shared_ptr<client::ReplicaClient> replicaClient =
            std::make_shared<client::ReplicaClient>(
                grpc::CreateChannel(remoteReplicaClientAddress,
                                    grpc::InsecureChannelCredentials()));

        std::shared_ptr<Replica> remoteReplica =
            std::make_shared<RemoteReplica>(std::move(replicaClient));

        replicaSet.emplace_back(std::move(remoteReplica));

        if (remoteReplicaConfig.id() != replica->getId()) {
          registry->registerReplica(remoteReplicaConfig.id(), remoteReplica);
        }
      }

      CHECK(replicaSet.size() == config.replica_set().size());
    }

    void makeSequencerServer() {
      const auto sequencerAddress =
          fmt::format(GRPC_SVC_ADDRESS_FORMAT,
                      config.sequencer_config().ip_address().host(),
                      config.sequencer_config().ip_address().port());
      sequencer = makeSequencer(config.sequencer_config().id(), replicaSet);

      sequencerServer = std::make_shared<server::SequencerServer>(sequencer);

      sequencerGRPCServer =
          durable_log::server::runRPCServer(
              sequencerAddress, {sequencerServer.get()},
              threadPoolExecutor.get(), config.sequencer_config().id())
              .get();
    }

    void makeVirtualLog() {
      virtualLog = std::make_shared<VirtualLogImpl>(
          "VIRTUAL_LOG_" + utils::UuidGenerator::instance().generate(),
          "appName", std::make_shared<NullSequencer>(), replicaSet,
          metadataStore, 1, registry);
    }

    void registerSequencers() const {
      // register local sequencer
      registry->registerSequencer(config.sequencer_config().id(), sequencer);

      // register remote sequencers
      for (const auto& remoteSequencerConfig : config.remote_sequencer_set()) {
        const auto remoteSequencerClientAddress = fmt::format(
            GRPC_SVC_ADDRESS_FORMAT, remoteSequencerConfig.ip_address().host(),
            remoteSequencerConfig.ip_address().port());
        std::shared_ptr<client::SequencerClient> sequencerClient =
            std::make_shared<client::SequencerClient>(
                grpc::CreateChannel(remoteSequencerClientAddress,
                                    grpc::InsecureChannelCredentials()));

        std::shared_ptr<Sequencer> remoteSequencer =
            std::make_shared<RemoteSequencer>(std::move(sequencerClient));

        registry->registerSequencer(remoteSequencerConfig.id(),
                                    remoteSequencer);
      }
    }
  };

  rk::projects::server::ServerConfig config_;
  rk::projects::durable_log::Options options_;
  std::unique_ptr<std::mutex> mtx_;
  std::shared_ptr<State> state_;
};

}  // namespace rk::projects::durable_log::server