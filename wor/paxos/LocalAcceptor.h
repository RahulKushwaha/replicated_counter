//
// Created by Rahul  Kushwaha on 6/3/23.
//

#pragma once
#include "Common.h"
#include "folly/experimental/coro/Task.h"
#include "persistence/KVStoreLite.h"
#include "wor/paxos/include/Acceptor.h"
#include "wor/paxos/proto/PaxosMessage.pb.h"
#include <cstdint>
#include <optional>
#include <string>

namespace rk::projects::paxos {

class LocalAcceptor : public Acceptor {
public:
  explicit LocalAcceptor(std::string id,
                         std::shared_ptr<persistence::KVStoreLite> kvStore);

  std::string getId() override;
  coro<std::variant<Promise, std::false_type>>
  prepare(std::string paxosInstanceId, Ballot ballot) override;
  coro<bool> accept(std::string paxosInstanceId, Proposal proposal) override;
  coro<bool> commit(std::string paxosInstanceId, BallotId ballotId) override;
  coro<std::optional<Promise>>
  getAcceptedValue(std::string paxosInstanceId) override;
  coro<std::optional<std::string>>
  getCommittedValue(std::string paxosInstanceId) override;

private:
  coro<internal::PaxosInstance> getOrCreate(std::string id);

private:
  std::string id_;
  std::shared_ptr<persistence::KVStoreLite> kvStore_;
  std::unique_ptr<std::mutex> mtx_;

  static constexpr char const *KEY_FORMAT = "PAXOS_DATA|{}";
};

} // namespace rk::projects::paxos