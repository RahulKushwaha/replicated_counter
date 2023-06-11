//
// Created by Rahul  Kushwaha on 6/18/23.
//

#pragma once
#include "folly/experimental/coro/Task.h"
#include "wor/paxos/Common.h"
#include <cstdint>
#include <optional>
#include <string>

namespace rk::projects::paxos {

class Acceptor {
public:
  virtual std::string getId() = 0;

  virtual coro<std::variant<Promise, std::false_type>>
  prepare(Ballot ballot) = 0;
  virtual coro<bool> accept(Proposal proposal) = 0;
  virtual coro<bool> commit(BallotId ballotId) = 0;

  virtual coro<std::optional<Promise>> getAcceptedValue() = 0;
  virtual coro<std::optional<std::string>> getCommittedValue() = 0;
};

} // namespace rk::projects::paxos