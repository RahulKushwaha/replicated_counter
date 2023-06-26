//
// Created by Rahul  Kushwaha on 6/12/23.
//
#include "wor/paxos/LocalAcceptor.h"
#include <gtest/gtest.h>

namespace rk::projects::paxos {

Ballot getNewBallot(std::int64_t id) {
  Ballot ballot{};
  ballot.id.set_major_id(0);
  ballot.id.set_minor_id(id);
  return ballot;
}

TEST(AcceptorTests, PrepareReturnsEmptyPromiseIfNotInitialized) {
  LocalAcceptor acceptor{"1"};
  auto promise = acceptor.prepare(getNewBallot(0)).semi().get();

  ASSERT_TRUE(std::holds_alternative<Promise>(promise));
}

TEST(AcceptorTests, PrepareReturnsFalseForSmallerOrEqualPromise) {
  LocalAcceptor acceptor{"1"};
  auto promise = acceptor.prepare(getNewBallot(10)).semi().get();

  ASSERT_TRUE(std::holds_alternative<Promise>(promise));

  for (int i = 0; i <= 10; i++) {
    auto p = acceptor.prepare(getNewBallot(i)).semi().get();
    ASSERT_TRUE(std::holds_alternative<std::false_type>(p));
  }
}

TEST(AcceptorTests, PrepareReturnsAcceptedValue) {
  LocalAcceptor acceptor{"1"};
  auto ballotId = getNewBallot(10);
  std::string value{"hello_world"};
  auto promise = acceptor.prepare(ballotId).semi().get();
  ASSERT_TRUE(std::holds_alternative<Promise>(promise));

  auto proposal = Proposal{};
  proposal.mutable_ballot_id()->CopyFrom(ballotId.id);
  proposal.set_value(value);

  auto acceptedValue = acceptor.accept(std::move(proposal)).semi().get();
  ASSERT_TRUE(acceptedValue);

  for (auto id = ballotId.id.minor_id() + 1; id < 100; id++) {
    auto prepareResult = acceptor.prepare(getNewBallot(id)).semi().get();
    ASSERT_TRUE(std::holds_alternative<Promise>(prepareResult));

    auto p = std::get<Promise>(prepareResult);
    ASSERT_TRUE(p.ballot_id() == ballotId.id);
    ASSERT_EQ(p.value(), value);
  }
}

TEST(AcceptorTests, AfterAcceptingValueOldProposalAreDenied) {
  LocalAcceptor acceptor{"1"};
  auto ballot{getNewBallot(10)};
  std::string value{"hello_world"};
  auto promise = acceptor.prepare(ballot).semi().get();
  ASSERT_TRUE(std::holds_alternative<Promise>(promise));

  auto proposal = Proposal{};
  proposal.mutable_ballot_id()->CopyFrom(ballot.id);
  proposal.set_value(value);

  auto acceptedValue = acceptor.accept(proposal).semi().get();
  ASSERT_TRUE(acceptedValue);

  for (int id = 0; id <= ballot.id.minor_id(); id++) {
    auto p = acceptor.prepare(getNewBallot(id)).semi().get();
    ASSERT_FALSE(std::get<std::false_type>(p));
  }
}

} // namespace rk::projects::paxos