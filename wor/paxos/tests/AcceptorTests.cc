//
// Created by Rahul  Kushwaha on 6/12/23.
//
#include "folly/experimental/TestUtil.h"
#include "persistence/RocksDbFactory.h"
#include "persistence/RocksKVStoreLite.h"
#include "wor/paxos/LocalAcceptor.h"
#include <gtest/gtest.h>

namespace rk::projects::paxos {

class AcceptorTests : public testing::TestWithParam<std::string> {
protected:
  std::shared_ptr<folly::test::TemporaryDirectory> tmpDir_;
  std::shared_ptr<rocksdb::DB> rocks_;
  std::shared_ptr<persistence::KVStoreLite> kvStore_;
  persistence::RocksDbFactory::RocksDbConfig config_{.createIfMissing = true};

  void SetUp() override {
    tmpDir_ = std::make_shared<folly::test::TemporaryDirectory>();
    config_.path = tmpDir_->path().string();
    rocks_ = persistence::RocksDbFactory::provideSharedPtr(config_);
    kvStore_ = std::make_shared<persistence::RocksKVStoreLite>(rocks_);
  }

  void TearDown() override {
    auto s = rocksdb::DestroyDB(config_.path, rocksdb::Options{});
    LOG(INFO) << "db destroy: " << config_.path << ", " << s.ToString();
  }
};

Ballot getNewBallot(std::int64_t id) {
  Ballot ballot{};
  ballot.id.set_major_id(0);
  ballot.id.set_minor_id(id);
  return ballot;
}

TEST_P(AcceptorTests, PrepareReturnsEmptyPromiseIfNotInitialized) {
  auto paxosInstanceId = GetParam();
  LocalAcceptor acceptor{"1", kvStore_};
  auto promise =
      acceptor.prepare(paxosInstanceId, getNewBallot(0)).semi().get();

  ASSERT_TRUE(std::holds_alternative<Promise>(promise));
}

TEST_P(AcceptorTests, PrepareReturnsFalseForSmallerOrEqualPromise) {
  auto paxosInstanceId = GetParam();
  LocalAcceptor acceptor{"1", kvStore_};
  auto promise =
      acceptor.prepare(paxosInstanceId, getNewBallot(10)).semi().get();

  ASSERT_TRUE(std::holds_alternative<Promise>(promise));

  for (int i = 0; i <= 10; i++) {
    auto p = acceptor.prepare(paxosInstanceId, getNewBallot(i)).semi().get();
    ASSERT_TRUE(std::holds_alternative<std::false_type>(p));
  }
}

TEST_P(AcceptorTests, PrepareReturnsAcceptedValue) {
  auto paxosInstanceId = GetParam();
  LocalAcceptor acceptor{"1", kvStore_};
  auto ballotId = getNewBallot(10);
  std::string value{"hello_world"};
  auto promise = acceptor.prepare(paxosInstanceId, ballotId).semi().get();
  ASSERT_TRUE(std::holds_alternative<Promise>(promise));

  auto proposal = Proposal{};
  proposal.mutable_ballot_id()->CopyFrom(ballotId.id);
  proposal.set_value(value);

  auto acceptedValue =
      acceptor.accept(paxosInstanceId, std::move(proposal)).semi().get();
  ASSERT_TRUE(acceptedValue);

  for (auto id = ballotId.id.minor_id() + 1; id < 100; id++) {
    auto prepareResult =
        acceptor.prepare(paxosInstanceId, getNewBallot(id)).semi().get();
    ASSERT_TRUE(std::holds_alternative<Promise>(prepareResult));

    auto p = std::get<Promise>(prepareResult);
    ASSERT_TRUE(p.ballot_id() == ballotId.id);
    ASSERT_EQ(p.value(), value);
  }
}

TEST_P(AcceptorTests, AfterAcceptingValueOldProposalAreDenied) {
  auto paxosInstanceId = GetParam();
  LocalAcceptor acceptor{"1", kvStore_};
  auto ballot{getNewBallot(10)};
  std::string value{"hello_world"};
  auto promise = acceptor.prepare(paxosInstanceId, ballot).semi().get();
  ASSERT_TRUE(std::holds_alternative<Promise>(promise));

  auto proposal = Proposal{};
  proposal.mutable_ballot_id()->CopyFrom(ballot.id);
  proposal.set_value(value);

  auto acceptedValue = acceptor.accept(paxosInstanceId, proposal).semi().get();
  ASSERT_TRUE(acceptedValue);

  for (int id = 0; id <= ballot.id.minor_id(); id++) {
    auto p = acceptor.prepare(paxosInstanceId, getNewBallot(id)).semi().get();
    ASSERT_FALSE(std::get<std::false_type>(p));
  }
}

INSTANTIATE_TEST_SUITE_P(AcceptorValueParameterizedTests, AcceptorTests,
                         testing::Values("id1", "id2", "id3", "id4", "id5",
                                         "id6", "id7", "id8"));

} // namespace rk::projects::paxos