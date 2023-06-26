//
// Created by Rahul  Kushwaha on 6/26/23.
//

#include "LocalAcceptor.h"

namespace rk::projects::paxos {

LocalAcceptor::LocalAcceptor(std::string id)
    : id_{std::move(id)}, paxosInstance_{} {
  paxosInstance_.mutable_promised_ballot_id()->set_major_id(-1);
  paxosInstance_.mutable_promised_ballot_id()->set_minor_id(-1);
  paxosInstance_.set_committed(false);
}

std::string LocalAcceptor::getId() { return id_; }

coro<std::variant<Promise, std::false_type>>
LocalAcceptor::prepare(Ballot ballot) {
  if (ballot.id <= paxosInstance_.promised_ballot_id()) {
    co_return std::false_type{};
  }

  paxosInstance_.mutable_promised_ballot_id()->CopyFrom(ballot.id);

  if (paxosInstance_.has_accepted()) {
    co_return paxosInstance_.accepted();
  }

  Promise promise{};
  promise.mutable_ballot_id()->CopyFrom(ballot.id);
  co_return promise;
}

coro<bool> LocalAcceptor::accept(Proposal proposal) {
  if (proposal.ballot_id() < paxosInstance_.promised_ballot_id()) {
    co_return false;
  }

  paxosInstance_.mutable_promised_ballot_id()->CopyFrom(proposal.ballot_id());

  paxosInstance_.mutable_accepted()->mutable_ballot_id()->CopyFrom(
      proposal.ballot_id());
  paxosInstance_.mutable_accepted()->set_value(proposal.value());
  co_return true;
}

coro<bool> LocalAcceptor::commit(BallotId ballotId) {
  if (!paxosInstance_.committed()) {
    paxosInstance_.set_committed(true);
    co_return true;
  }

  co_return false;
}

coro<std::optional<Promise>> LocalAcceptor::getAcceptedValue() {
  co_return paxosInstance_.accepted();
}

coro<std::optional<std::string>> LocalAcceptor::getCommittedValue() {
  if (paxosInstance_.committed()) {
    co_return paxosInstance_.accepted().value();
  }

  co_return {};
}

} // namespace rk::projects::paxos