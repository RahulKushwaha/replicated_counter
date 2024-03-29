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

  virtual coro<std::variant<Promise, std::false_type>> prepare(
      std::string paxosInstanceId, Ballot ballot) = 0;
  virtual coro<bool> accept(std::string paxosInstanceId, Proposal proposal) = 0;
  virtual coro<bool> commit(std::string paxosInstanceId, BallotId ballotId) = 0;

  virtual coro<std::optional<Promise>> getAcceptedValue(
      std::string paxosInstanceId) = 0;
  virtual coro<std::optional<std::string>> getCommittedValue(
      std::string paxosInstanceId) = 0;
};

}  // namespace rk::projects::paxos