//
// Created by Rahul  Kushwaha on 1/5/23.
//

#include <gtest/gtest.h>

#include "../InMemoryMetadataStore.h"

namespace rk::project::counter {

TEST(MetadataStoreTest, getFromEmptyStore) {
  InMemoryMetadataStore store;
  auto result = store.getConfig(0);

  ASSERT_EQ(result.startindex(), 0);
  ASSERT_EQ(result.versionid(), 0);
}

}
