//
// Created by Rahul  Kushwaha on 6/3/23.
//

#pragma once
#include "Common.h"
#include "folly/experimental/coro/Task.h"
#include "wor/paxos/include/Acceptor.h"
#include <cstdint>
#include <optional>
#include <string>

namespace rk::projects::paxos {

class LocalAcceptor : public Acceptor {
public:
  explicit LocalAcceptor(std::string id)
      : id_{std::move(id)}, promisedBallotId_{-1}, accepted_{},
        committed_{false} {}

  std::string getId() override { return id_; }

  coro<std::variant<Promise, std::false_type>> prepare(Ballot ballot) override {
    if (ballot.id <= promisedBallotId_) {
      co_return std::false_type{};
    }

    promisedBallotId_ = ballot.id;

    if (accepted_.value.has_value()) {
      co_return accepted_;
    }

    co_return Promise{.id = promisedBallotId_};
  }

  coro<bool> accept(Proposal proposal) override {
    if (proposal.id < promisedBallotId_) {
      co_return false;
    }

    promisedBallotId_ = proposal.id;
    accepted_ = Promise{.id = proposal.id, .value = proposal.value};
    co_return true;
  }

  coro<bool> commit(BallotId ballotId) override {
    if (!committed_) {
      committed_ = true;
      co_return true;
    }

    co_return false;
  }

  coro<std::optional<Promise>> getAcceptedValue() override {
    co_return accepted_;
  }

  coro<std::optional<std::string>> getCommittedValue() override {
    if (committed_) {
      co_return accepted_.value;
    }

    co_return {};
  }

private:
  std::string id_;
  BallotId promisedBallotId_;
  Promise accepted_;
  bool committed_;
};

} // namespace rk::projects::paxos