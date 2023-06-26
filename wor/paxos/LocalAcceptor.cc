//
// Created by Rahul  Kushwaha on 6/26/23.
//

#include "LocalAcceptor.h"

namespace rk::projects::paxos {

LocalAcceptor::LocalAcceptor(std::string id,
                             std::shared_ptr<wor::KVStoreLite> persistence)
    : id_{std::move(id)}, lookup_{}, persistence_{std::move(persistence)} {}

std::string LocalAcceptor::getId() { return id_; }

coro<std::variant<Promise, std::false_type>>
LocalAcceptor::prepare(Ballot ballot) {
  auto paxosInstance = getOrCreate(ballot.id.paxos_instance_id());
  if (ballot.id <= paxosInstance->promised_ballot_id()) {
    co_return std::false_type{};
  }

  paxosInstance->mutable_promised_ballot_id()->CopyFrom(ballot.id);
  co_await persistence_->put(paxosInstance->id(),
                             paxosInstance->SerializeAsString());

  if (paxosInstance->has_accepted()) {
    co_return paxosInstance->accepted();
  }

  Promise promise{};
  promise.mutable_ballot_id()->CopyFrom(ballot.id);
  co_return promise;
}

coro<bool> LocalAcceptor::accept(Proposal proposal) {
  auto paxosInstance = getOrCreate(proposal.ballot_id().paxos_instance_id());

  if (proposal.ballot_id() < paxosInstance->promised_ballot_id()) {
    co_return false;
  }

  paxosInstance->mutable_promised_ballot_id()->CopyFrom(proposal.ballot_id());

  paxosInstance->mutable_accepted()->mutable_ballot_id()->CopyFrom(
      proposal.ballot_id());
  paxosInstance->mutable_accepted()->set_value(proposal.value());

  co_await persistence_->put(paxosInstance->id(),
                             paxosInstance->SerializeAsString());
  co_return true;
}

coro<bool> LocalAcceptor::commit(BallotId ballotId) {
  auto paxosInstance = getOrCreate(ballotId.paxos_instance_id());

  if (!paxosInstance->committed()) {
    paxosInstance->set_committed(true);
    co_await persistence_->put(paxosInstance->id(),
                               paxosInstance->SerializeAsString());
    co_return true;
  }

  co_return false;
}

coro<std::optional<Promise>>
LocalAcceptor::getAcceptedValue(std::string paxosInstanceId) {
  if (auto itr = lookup_.find(paxosInstanceId); itr != lookup_.end()) {
    co_return itr->second->accepted();
  }

  co_return {};
}

coro<std::optional<std::string>>
LocalAcceptor::getCommittedValue(std::string paxosInstanceId) {
  if (auto itr = lookup_.find(paxosInstanceId); itr != lookup_.end()) {
    if (auto &paxosInstance = *itr->second; paxosInstance.committed()) {
      co_return paxosInstance.accepted().value();
    }
  }

  co_return {};
}

internal::PaxosInstance *
LocalAcceptor::getOrCreate(std::string paxosInstanceId) {
  internal::PaxosInstance *paxosInstance;
  if (auto itr = lookup_.find(paxosInstanceId); itr != lookup_.end()) {
    paxosInstance = itr->second.get();
  } else {
    auto uniquePaxosInstance = std::make_unique<internal::PaxosInstance>();
    uniquePaxosInstance->set_id(paxosInstanceId);
    uniquePaxosInstance->mutable_promised_ballot_id()->set_major_id(-1);
    uniquePaxosInstance->mutable_promised_ballot_id()->set_minor_id(-1);

    paxosInstance = uniquePaxosInstance.get();
    lookup_.emplace(paxosInstanceId, std::move(uniquePaxosInstance));
  }

  return paxosInstance;
}

} // namespace rk::projects::paxos