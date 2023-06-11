//
// Created by Rahul  Kushwaha on 6/19/23.
//
#pragma once
#include "folly/experimental/coro/Task.h"
#include "wor/paxos/Common.h"
#include <cstdint>
#include <optional>
#include <string>

namespace rk::projects::paxos {

class Proposer1 {
public:
  virtual coro<bool> prepare(BallotId ballotId) = 0;
  virtual coro<bool> propose(BallotId ballotId, std::string value) = 0;

  virtual coro<std::optional<std::string>> getCommittedValue() = 0;
};

} // namespace rk::projects::paxos