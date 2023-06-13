//
// Created by Rahul  Kushwaha on 3/5/23.
//

#include "../UuidGenerator.h"
#include <gtest/gtest.h>
#include <unordered_set>

namespace rk::projects::utils {

TEST(UuidGeneratorTests, createUuid) {
  ASSERT_EQ(UuidGenerator::instance().generate().size(), 36);
}

TEST(UuidGeneratorTests, createUuidMultipleAndAreUnique) {
  int limit = 100;
  std::unordered_set<std::string> uuids;
  for (int i = 0; i < limit; i++) {
    auto uuid = UuidGenerator::instance().generate();
    ASSERT_EQ(uuid.size(), 36);
    uuids.emplace(std::move(uuid));
  }

  ASSERT_EQ(uuids.size(), limit);
}

} // namespace rk::projects::utils