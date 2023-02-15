//
// Created by Rahul  Kushwaha on 1/5/23.
//

#include <gtest/gtest.h>

#include "../InMemoryMetadataStore.h"

namespace rk::project::counter {

TEST(MetadataStoreTest, getFromEmptyStore) {
  InMemoryMetadataStore store;
  auto result = store.getConfig(0);

  ASSERT_FALSE(result.has_value());
}

TEST(MetadataStoreTest, InitialVersionReturnedOnEmptyStoreIs0) {
  InMemoryMetadataStore store;
  ASSERT_EQ(store.getCurrentVersionId(), 0);
}

TEST(MetadataStoreTest, addToEmptyStore) {
  // Adding first entry should always have version_id as 0;
  {
    InMemoryMetadataStore store;
    MetadataConfig config;
    ASSERT_THROW(store.compareAndAppendRange(4, config),
                 OptimisticConcurrencyException);
  }

  {
    InMemoryMetadataStore store;
    MetadataConfig config;
    ASSERT_THROW(store.compareAndAppendRange(10, config),
                 OptimisticConcurrencyException);
  }
}

TEST(MetadataStoreTest, addToAlreadyExistingStore) {
  InMemoryMetadataStore store;

  {
    MetadataConfig config;
    config.set_versionid(1);
    config.set_previousversionid(0);
    config.set_startindex(500);
    config.set_endindex(1000);

    ASSERT_NO_THROW(store.compareAndAppendRange(0, config));
  }

  // Add the next version.
  {
    MetadataConfig config;
    config.set_versionid(2);
    config.set_previousversionid(1);
    config.set_startindex(1500);
    config.set_endindex(2000);

    ASSERT_NO_THROW(store.compareAndAppendRange(1, config));
  }

  // Add the next version.
  {
    MetadataConfig config;
    config.set_versionid(3);
    config.set_previousversionid(2);
    config.set_startindex(4500);
    config.set_endindex(5000);

    ASSERT_NO_THROW(store.compareAndAppendRange(2, config));
  }
}

TEST(MetadataStoreTest, addToAlreadyExistingStoreSameConfig) {
  InMemoryMetadataStore store;

  {
    MetadataConfig config;
    config.set_versionid(1);
    config.set_previousversionid(0);
    config.set_startindex(500);
    config.set_endindex(1000);

    ASSERT_NO_THROW(store.compareAndAppendRange(0, config));
  }

  // Add the next version.
  {
    MetadataConfig config;
    config.set_versionid(2);
    config.set_previousversionid(1);
    config.set_startindex(1500);
    config.set_endindex(2000);

    ASSERT_NO_THROW(store.compareAndAppendRange(1, config));
  }

  // Add the next version.
  {
    MetadataConfig config;
    config.set_versionid(2);
    config.set_previousversionid(1);
    config.set_startindex(1500);
    config.set_endindex(2000);

    ASSERT_THROW(store.compareAndAppendRange(1, config),
                 OptimisticConcurrencyException);
  }
}

TEST(MetadataStoreTest, addToAlreadyExistingStoreSkippingConfigs) {
  InMemoryMetadataStore store;

  {
    MetadataConfig config;
    config.set_versionid(1);
    config.set_previousversionid(0);
    config.set_startindex(500);
    config.set_endindex(1000);

    ASSERT_NO_THROW(store.compareAndAppendRange(0, config));
  }

  // Add the next version.
  {
    MetadataConfig config;
    config.set_versionid(2);
    config.set_previousversionid(1);
    config.set_startindex(1500);
    config.set_endindex(2000);

    ASSERT_NO_THROW(store.compareAndAppendRange(1, config));
  }

  // Add the skipped version.
  {
    MetadataConfig config;
    config.set_versionid(4);
    config.set_previousversionid(2);
    config.set_startindex(1500);
    config.set_endindex(2000);

    ASSERT_THROW(store.compareAndAppendRange(2, config),
                 OptimisticConcurrencyException);
  }
}

TEST(MetadataStoreTest, getUsingLogId) {
  InMemoryMetadataStore store;

  {
    MetadataConfig config;
    config.set_versionid(1);
    config.set_previousversionid(2);
    config.set_startindex(1500);
    config.set_endindex(2000);

    ASSERT_NO_THROW(store.compareAndAppendRange(0, config));
  }

  {
    auto config = store.getConfigUsingLogId(1000);
    ASSERT_FALSE(config.has_value());
  }

  {
    auto config = store.getConfigUsingLogId(1500);
    ASSERT_TRUE(config.has_value());
  }

  {
    auto config = store.getConfigUsingLogId(1600);
    ASSERT_TRUE(config.has_value());
  }

  // logId == 2000 should not exist.
  {
    auto config = store.getConfigUsingLogId(2000);
    ASSERT_FALSE(config.has_value());
  }
}

TEST(MetadataStoreTest, getUsingLogIdWhenEmptyConfigsArePresent) {
  InMemoryMetadataStore store;

  {
    MetadataConfig config;
    config.set_versionid(1);
    config.set_previousversionid(2);
    config.set_startindex(1500);
    config.set_endindex(2000);

    ASSERT_NO_THROW(store.compareAndAppendRange(0, config));
  }

  {
    MetadataConfig config;
    config.set_versionid(2);
    config.set_previousversionid(2);
    config.set_startindex(2000);
    config.set_endindex(2000);

    ASSERT_NO_THROW(store.compareAndAppendRange(1, config));
  }

  {
    MetadataConfig config;
    config.set_versionid(3);
    config.set_previousversionid(2);
    config.set_startindex(2000);
    config.set_endindex(2000);

    ASSERT_NO_THROW(store.compareAndAppendRange(2, config));
  }

  {
    auto config = store.getConfigUsingLogId(2000);
    ASSERT_FALSE(config.has_value());
  }

  {
    MetadataConfig config;
    config.set_versionid(4);
    config.set_previousversionid(2);
    config.set_startindex(2000);
    config.set_endindex(2001);

    ASSERT_NO_THROW(store.compareAndAppendRange(3, config));
  }

  // Now we should be able to fine 2000.
  {
    auto config = store.getConfigUsingLogId(2000);
    ASSERT_TRUE(config.has_value());
  }
}

}
