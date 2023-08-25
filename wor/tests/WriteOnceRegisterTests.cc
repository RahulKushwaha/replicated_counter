//
// Created by Rahul  Kushwaha on 6/7/23.
//

#include "persistence/RocksDbFactory.h"
#include "persistence/RocksKVStoreLite.h"
#include "wor/inmemory/InMemoryWriteOnceRegister.h"
#include "wor/paxos/LocalAcceptor.h"
#include "wor/paxos/PaxosWriteOnceRegister.h"
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <vector>

namespace rk::projects::wor {
using namespace paxos;

static std::int32_t majorId = 1;

std::shared_ptr<PaxosWriteOnceRegister> createPaxosRegister() {
  int members{5};

  std::vector<std::shared_ptr<Acceptor>> acceptors;
  for (int i = 0; i < members; i++) {
    std::string acceptorId = fmt::format("acceptor_{}", i);
    persistence::RocksDbFactory::RocksDbConfig config{
        .path = fmt::format(
            "/tmp/paxos_tests/paxos_acceptor_tests{}_{}",
            std::chrono::system_clock::now().time_since_epoch().count(), i),
        .createIfMissing = true};
    auto rocks = persistence::RocksDbFactory::provideSharedPtr(config);
    auto kvStore =
        std::make_shared<persistence::RocksKVStoreLite>(std::move(rocks));
    acceptors.emplace_back(
        std::make_shared<LocalAcceptor>(acceptorId, std::move(kvStore)));
  }

  auto proposer = std::make_shared<ProposerImpl>(members, acceptors);
  auto paxosRegister =
      std::make_shared<PaxosWriteOnceRegister>(majorId++, proposer);
  return paxosRegister;
}

class WorTests : public testing::TestWithParam<
                     std::function<std::shared_ptr<WriteOnceRegister>()>> {};

TEST_P(WorTests, LockIdsAreIncreme) {
  auto wor = GetParam()();

  std::vector<LockId> lockIds;
  for (int i = 0; i < 100; i++) {
    auto lockId = wor->lock().semi().get();
    ASSERT_TRUE(lockId.has_value());
    lockIds.emplace_back(lockId.value());
  }

  ASSERT_TRUE(std::is_sorted(lockIds.begin(), lockIds.end()));
}

TEST_P(WorTests, ReadFromUnCommittedReturnsError) {
  auto wor = GetParam()();
  auto result = wor->read().semi().get();

  ASSERT_TRUE(std::holds_alternative<WriteOnceRegister::ReadError>(result));

  auto readError = std::get<WriteOnceRegister::ReadError>(result);
  ASSERT_EQ(WriteOnceRegister::ReadError::NOT_WRITTEN, readError);
}

TEST_P(WorTests, WriteWithHigherLockValueSucceeds) {
  auto wor = GetParam()();

  // Acquire a bunch of locks.
  std::vector<LockId> lockIds;
  for (int i = 0; i < 100; i++) {
    auto lockId = wor->lock().semi().get();
    ASSERT_TRUE(lockId.has_value());
    lockIds.emplace_back(lockId.value());
  }

  auto lockId = wor->lock().semi().get();
  ASSERT_TRUE(lockId.has_value());

  // All the locks acquired previously should fail.
  for (auto prevLockId : lockIds) {
    auto writeResult = wor->write(prevLockId, "Hello World").semi().get();
    ASSERT_FALSE(writeResult);

    auto readResult = wor->read().semi().get();
    ASSERT_TRUE(
        std::holds_alternative<WriteOnceRegister::ReadError>(readResult));

    auto readError = std::get<WriteOnceRegister::ReadError>(readResult);
    ASSERT_EQ(WriteOnceRegister::ReadError::NOT_WRITTEN, readError);
  }

  auto result = wor->write(lockId.value(), "Hello World 1").semi().get();
  ASSERT_TRUE(result);

  ASSERT_EQ("Hello World 1", std::get<std::string>(wor->read().semi().get()));
}

TEST_P(WorTests, WriteIsAllowedOnlyOnce) {
  auto payload{"Hello World"};
  auto wor = GetParam()();
  auto lockId = wor->lock().semi().get();
  ASSERT_TRUE(lockId.has_value());

  auto result = wor->write(lockId.value(), payload).semi().get();
  ASSERT_TRUE(result);

  ASSERT_EQ(payload, std::get<std::string>(wor->read().semi().get()));

  // Try writing it multiple times, and the result should be the same every
  // time.
  for (int i = 0; i < 100; i++) {
    auto newPayload = fmt::format("Hello World {}", i);

    lockId = wor->lock().semi().get();
    ASSERT_TRUE(lockId.has_value());

    result = wor->write(lockId.value(), newPayload).semi().get();
    ASSERT_FALSE(result);

    ASSERT_EQ(payload, std::get<std::string>(wor->read().semi().get()));
  }
}

// TODO(Rahul): Refactor to use setup method of test to create test. Temporary
// hack to use lambda for lazy evaluation.
INSTANTIATE_TEST_SUITE_P(
    WorParameterizedTests, WorTests,
    testing::Values(
        []() { return std::make_shared<InMemoryWriteOnceRegister>(); },
        []() { return createPaxosRegister(); }));

} // namespace rk::projects::wor