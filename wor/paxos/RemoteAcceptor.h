//
// Created by Rahul  Kushwaha on 7/23/23.
//

#pragma once
#include "wor/paxos/client/AcceptorClient.h"
#include "wor/paxos/include/Acceptor.h"

namespace rk::projects::paxos {

class RemoteAcceptor : public paxos::Acceptor {
public:
  explicit RemoteAcceptor(std::shared_ptr<client::AcceptorClient> client)
      : remote_{std::move(client)} {}

  std::string getId() override { return remote_->getId(); }

  coro<std::variant<Promise, std::false_type>>
  prepare(std::string paxosInstanceId, Ballot ballot) override {
    co_return co_await remote_->prepare(std::move(paxosInstanceId),
                                        std::move(ballot));
  }

  coro<bool> accept(std::string paxosInstanceId, Proposal proposal) override {
    co_return co_await remote_->accept(std::move(paxosInstanceId),
                                       std::move(proposal));
  }

  coro<bool> commit(std::string paxosInstanceId, BallotId ballotId) override {
    co_return co_await remote_->commit(std::move(paxosInstanceId),
                                       std::move(ballotId));
  }

  coro<std::optional<Promise>>
  getAcceptedValue(std::string paxosInstanceId) override {
    co_return co_await remote_->getAcceptedValue(std::move(paxosInstanceId));
  }

  coro<std::optional<std::string>>
  getCommittedValue(std::string paxosInstanceId) override {
    co_return co_await remote_->getCommittedValue(std::move(paxosInstanceId));
  }

private:
  std::shared_ptr<client::AcceptorClient> remote_;
};

} // namespace rk::projects::paxos