//
// Created by Rahul  Kushwaha on 6/26/23.
//

#include "LocalAcceptor.h"

namespace rk::projects::paxos {

LocalAcceptor::LocalAcceptor(std::string id,
                             std::shared_ptr<persistence::KVStoreLite> kvStore)
    : id_{std::move(id)},
      kvStore_{std::move(kvStore)},
      mtx_{std::make_unique<std::mutex>()} {}

std::string LocalAcceptor::getId() {
  return id_;
}

coro<std::variant<Promise, std::false_type>> LocalAcceptor::prepare(
    std::string paxosInstanceId, Ballot ballot) {
  std::lock_guard lk{*mtx_};

  auto paxosInstance = co_await getOrCreate(paxosInstanceId);
  if (ballot.id <= paxosInstance.promised_ballot_id()) {
    co_return std::false_type{};
  }

  paxosInstance.mutable_promised_ballot_id()->CopyFrom(ballot.id);
  bool result =
      co_await kvStore_->put(fmt::format(KEY_FORMAT, paxosInstance.id()),
                             paxosInstance.SerializeAsString());

  if (!result) {
    throw std::runtime_error{"storage write failed"};
  }

  if (paxosInstance.has_accepted()) {
    co_return paxosInstance.accepted();
  }

  Promise promise{};
  promise.mutable_ballot_id()->CopyFrom(ballot.id);
  co_return promise;
}

coro<bool> LocalAcceptor::accept(std::string paxosInstanceId,
                                 Proposal proposal) {
  std::lock_guard lk{*mtx_};

  auto paxosInstance = co_await getOrCreate(paxosInstanceId);

  if (proposal.ballot_id() < paxosInstance.promised_ballot_id()) {
    co_return false;
  }

  paxosInstance.mutable_promised_ballot_id()->CopyFrom(proposal.ballot_id());

  paxosInstance.mutable_accepted()->mutable_ballot_id()->CopyFrom(
      proposal.ballot_id());
  paxosInstance.mutable_accepted()->set_value(proposal.value());

  bool result =
      co_await kvStore_->put(fmt::format(KEY_FORMAT, paxosInstance.id()),
                             paxosInstance.SerializeAsString());

  if (!result) {
    throw std::runtime_error{"storage write failed"};
  }

  co_return true;
}

coro<bool> LocalAcceptor::commit(std::string paxosInstanceId,
                                 BallotId ballotId) {
  std::lock_guard lk{*mtx_};

  auto paxosInstance = co_await getOrCreate(paxosInstanceId);

  if (!paxosInstance.committed()) {
    paxosInstance.set_committed(true);

    bool result =
        co_await kvStore_->put(fmt::format(KEY_FORMAT, paxosInstance.id()),
                               paxosInstance.SerializeAsString());

    if (!result) {
      throw std::runtime_error{"storage write failed"};
    }

    co_return true;
  }

  co_return false;
}

coro<std::optional<Promise>> LocalAcceptor::getAcceptedValue(
    std::string paxosInstanceId) {
  std::lock_guard lk{*mtx_};

  auto paxosInstance = co_await getOrCreate(paxosInstanceId);
  co_return paxosInstance.accepted();
}

coro<std::optional<std::string>> LocalAcceptor::getCommittedValue(
    std::string paxosInstanceId) {
  std::lock_guard lk{*mtx_};

  auto paxosInstance = co_await getOrCreate(paxosInstanceId);
  if (paxosInstance.committed()) {
    co_return paxosInstance.accepted().value();
  }

  co_return {};
}

coro<internal::PaxosInstance> LocalAcceptor::getOrCreate(std::string id) {
  std::optional<std::string> result =
      co_await kvStore_->get(fmt::format(KEY_FORMAT, id));
  if (result.has_value()) {
    internal::PaxosInstance paxosInstance{};
    bool parseResult = paxosInstance.ParseFromString(result.value());

    if (!parseResult) {
      throw std::runtime_error{"paxos instance could not be deserialized"};
    }

    co_return paxosInstance;
  }

  internal::PaxosInstance paxosInstance{};
  paxosInstance.set_id(id);
  paxosInstance.mutable_promised_ballot_id()->set_major_id(-1);
  paxosInstance.mutable_promised_ballot_id()->set_minor_id(-1);
  paxosInstance.set_committed(false);

  auto putResult = co_await kvStore_->put(fmt::format(KEY_FORMAT, id),
                                          paxosInstance.SerializeAsString());

  if (!putResult) {
    throw std::runtime_error{"storage error"};
  }

  co_return paxosInstance;
}

}  // namespace rk::projects::paxos