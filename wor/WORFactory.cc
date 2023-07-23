//
// Created by Rahul  Kushwaha on 7/8/23.
//

#include "WORFactory.h"
#include "persistence/RocksDbFactory.h"
#include "persistence/RocksKVStoreLite.h"
#include "wor/paxos/LocalAcceptor.h"
#include "wor/paxos/PaxosWriteOnceRegister.h"
#include "wor/paxos/ProposerImpl.h"
#include "wor/paxos/RemoteAcceptor.h"
#include "wor/paxos/client/AcceptorClient.h"

namespace rk::projects::wor {

std::unique_ptr<WriteOnceRegister> makePaxosWor(WorId worId,
                                                PaxosWorConfig paxosWorConfig) {
  auto members{paxosWorConfig.totalMembers()};

  std::vector<std::shared_ptr<paxos::Acceptor>> acceptors;
  for (const auto &replica : paxosWorConfig.getLocalConfig()) {
    std::string acceptorId = fmt::format("acceptor_{}", replica.id());
    persistence::RocksDbFactory::RocksDbConfig config{
        .path = fmt::format(
            "/tmp/paxos/paxos_acceptor{}_{}",
            std::chrono::system_clock::now().time_since_epoch().count(),
            replica.id()),
        .createIfMissing = true};
    auto rocks = persistence::RocksDbFactory::provideSharedPtr(config);
    auto kvStore =
        std::make_shared<persistence::RocksKVStoreLite>(std::move(rocks));

    acceptors.emplace_back(
        std::make_shared<paxos::LocalAcceptor>(acceptorId, std::move(kvStore)));
  }

  for (const auto &remote : paxosWorConfig.getRemoteConfigs()) {
    auto acceptorClient = std::make_shared<paxos::client::AcceptorClient>(
        grpc::CreateChannel(fmt::format("{}:{}", remote.ip_address().host(),
                                        remote.ip_address().port()),
                            grpc::InsecureChannelCredentials()));

    auto remoteAcceptor =
        std::make_shared<paxos::RemoteAcceptor>(std::move(acceptorClient));

    acceptors.emplace_back(std::move(remoteAcceptor));
  }

  auto proposer = std::make_shared<paxos::ProposerImpl>(members, acceptors);
  return std::make_unique<paxos::PaxosWriteOnceRegister>(worId,
                                                         std::move(proposer));
}

std::unique_ptr<WriteOnceRegisterChain> makeChainUsingInMemoryWor() {
  auto worFactory = [](WorId worId) {
    return std::make_shared<InMemoryWriteOnceRegister>();
  };
  return std::make_unique<WriteOnceRegisterChainImpl>(std::move(worFactory));
}

std::unique_ptr<WriteOnceRegisterChain> makeChainUsingPaxosWor() {
  int members{5};

  std::vector<std::shared_ptr<paxos::Acceptor>> acceptors;
  for (int i = 0; i < members; i++) {
    std::string acceptorId = fmt::format("acceptor_{}", i);
    persistence::RocksDbFactory::RocksDbConfig config{
        .path = fmt::format(
            "/tmp/paxos_tests/paxos_acceptor_tests{}_{}",
            std::chrono::system_clock::now().time_since_epoch().count(), i),
        .createIfMissing = true};
    auto rocks = persistence::RocksDbFactory::provideSharedPtr(config);
    auto kvStore =
        std::make_shared<persistence::RocksKVStoreLite>(std::move(rocks));
    acceptors.emplace_back(
        std::make_shared<paxos::LocalAcceptor>(acceptorId, std::move(kvStore)));
  }

  auto proposer = std::make_shared<paxos::ProposerImpl>(members, acceptors);
  auto worFactory = [proposer](WorId worId) {
    return std::make_shared<paxos::PaxosWriteOnceRegister>(worId, proposer);
  };

  return std::make_unique<WriteOnceRegisterChainImpl>(worFactory);
}

} // namespace rk::projects::wor