//
// Created by Rahul  Kushwaha on 6/3/23.
//

#pragma once
#include "Common.h"
#include "folly/experimental/coro/Task.h"
#include "wor/KVStoreLite.h"
#include "wor/paxos/include/Acceptor.h"
#include "wor/paxos/proto/PaxosMessage.pb.h"
#include <cstdint>
#include <optional>
#include <string>

namespace rk::projects::paxos {

class LocalAcceptor : public Acceptor {
public:
  explicit LocalAcceptor(std::string id);

  std::string getId() override;
  coro<std::variant<Promise, std::false_type>> prepare(Ballot ballot) override;
  coro<bool> accept(Proposal proposal) override;
  coro<bool> commit(BallotId ballotId) override;
  coro<std::optional<Promise>> getAcceptedValue() override;
  coro<std::optional<std::string>> getCommittedValue() override;

private:
  std::string id_;
  internal::PaxosInstance paxosInstance_;
};

} // namespace rk::projects::paxos