//
// Created by Rahul  Kushwaha on 6/7/23.
//

#include <gtest/gtest.h>
#include <fmt/format.h>
#include <vector>
#include "wor/inmemory/InMemoryWriteOnceRegister.h"

namespace rk::projects::wor {

TEST(InMemoryWriteOnceRegisterTests, LockIdsAreIncreme) {
  auto wor = InMemoryWriteOnceRegister{};

  std::vector<LockId> lockIds;
  for (int i = 0; i < 100; i++) {
    auto lockId = wor.lock();
    ASSERT_TRUE(lockId.has_value());
    lockIds.emplace_back(lockId.value());
  }

  ASSERT_TRUE(std::is_sorted(lockIds.begin(), lockIds.end()));
}

TEST(InMemoryWriteOnceRegisterTests, ReadFromUnCommittedReturnsError) {
  auto wor = InMemoryWriteOnceRegister{};
  auto result = wor.read();

  ASSERT_TRUE(std::holds_alternative<WriteOnceRegister::ReadError>(result));

  auto readError = std::get<WriteOnceRegister::ReadError>(result);
  ASSERT_EQ(WriteOnceRegister::ReadError::NOT_WRITTEN, readError);
}

TEST(InMemoryWriteOnceRegisterTests, WriteWithHigherLockValueSucceeds) {
  auto wor = InMemoryWriteOnceRegister{};

  // Acquire a bunch of locks.
  std::vector<LockId> lockIds;
  for (int i = 0; i < 100; i++) {
    auto lockId = wor.lock();
    ASSERT_TRUE(lockId.has_value());
    lockIds.emplace_back(lockId.value());
  }

  auto lockId = wor.lock();
  ASSERT_TRUE(lockId.has_value());

  // All the locks acquired previously should fail.
  for (auto prevLockId: lockIds) {
    auto writeResult = wor.write(prevLockId, "Hello World");
    ASSERT_FALSE(writeResult);

    auto readResult = wor.read();
    ASSERT_TRUE(std::holds_alternative<WriteOnceRegister::ReadError>(readResult));

    auto readError = std::get<WriteOnceRegister::ReadError>(readResult);
    ASSERT_EQ(WriteOnceRegister::ReadError::NOT_WRITTEN, readError);
  }

  auto result = wor.write(lockId.value(), "Hello World 1");
  ASSERT_TRUE(result);

  ASSERT_EQ("Hello World 1", std::get<std::string>(wor.read()));
}

TEST(InMemoryWriteOnceRegisterTests, WriteIsAllowedOnlyOnce) {
  auto payload{"Hello World"};
  auto wor = InMemoryWriteOnceRegister();
  auto lockId = wor.lock();
  ASSERT_TRUE(lockId.has_value());

  auto result = wor.write(lockId.value(), payload);
  ASSERT_TRUE(result);

  ASSERT_EQ(payload, std::get<std::string>(wor.read()));

  // Try writing it multiple times, and the result should be the same every time.
  for (int i = 0; i < 100; i++) {
    auto newPayload = fmt::format("Hello World {}", i);

    lockId = wor.lock();
    ASSERT_TRUE(lockId.has_value());

    result = wor.write(lockId.value(), newPayload);
    ASSERT_FALSE(result);

    ASSERT_EQ(payload, std::get<std::string>(wor.read()));
  }
}

}