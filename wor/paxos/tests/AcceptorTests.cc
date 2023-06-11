//
// Created by Rahul  Kushwaha on 6/12/23.
//
#include "wor/paxos/LocalAcceptor.h"
#include <gtest/gtest.h>

namespace rk::projects::paxos {

TEST(AcceptorTests, PrepareReturnsEmptyPromiseIfNotInitialized) {
  LocalAcceptor acceptor{"1"};
  auto promise = acceptor.prepare(Ballot{0}).semi().get();

  ASSERT_TRUE(std::holds_alternative<Promise>(promise));
}

TEST(AcceptorTests, PrepareReturnsFalseForSmallerOrEqualPromise) {
  LocalAcceptor acceptor{"1"};
  auto promise = acceptor.prepare(Ballot{10}).semi().get();

  ASSERT_TRUE(std::holds_alternative<Promise>(promise));

  for (int i = 0; i <= 10; i++) {
    auto p = acceptor.prepare(Ballot{.id = i}).semi().get();
    ASSERT_TRUE(std::holds_alternative<std::false_type>(p));
  }
}

TEST(AcceptorTests, PrepareReturnsAcceptedValue) {
  LocalAcceptor acceptor{"1"};
  auto ballotId{10};
  std::string value{"hello_world"};
  auto promise = acceptor.prepare(Ballot{ballotId}).semi().get();
  ASSERT_TRUE(std::holds_alternative<Promise>(promise));

  auto acceptedValue = acceptor.accept(Proposal{ballotId, value}).semi().get();
  ASSERT_TRUE(acceptedValue);

  for (int id = ballotId + 1; id < 100; id++) {
    auto prepareResult = acceptor.prepare(Ballot{id}).semi().get();
    ASSERT_TRUE(std::holds_alternative<Promise>(prepareResult));

    auto p = std::get<Promise>(prepareResult);
    ASSERT_EQ(p.id, ballotId);
    ASSERT_EQ(p.value, value);
  }
}

TEST(AcceptorTests, AfterAcceptingValueOldProposalAreDenied) {
  LocalAcceptor acceptor{"1"};
  auto ballotId{10};
  std::string value{"hello_world"};
  auto promise = acceptor.prepare(Ballot{ballotId}).semi().get();
  ASSERT_TRUE(std::holds_alternative<Promise>(promise));

  auto acceptedValue = acceptor.accept(Proposal{ballotId, value}).semi().get();
  ASSERT_TRUE(acceptedValue);

  for (int id = 0; id <= ballotId; id++) {
    auto p = acceptor.prepare(Ballot{id}).semi().get();
    ASSERT_FALSE(std::get<std::false_type>(p));
  }
}

} // namespace rk::projects::paxos