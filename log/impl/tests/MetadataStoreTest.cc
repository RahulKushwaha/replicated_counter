//
// Created by Rahul  Kushwaha on 1/5/23.
//

#include <gtest/gtest.h>

#include "log/impl/InMemoryMetadataStore.h"

namespace rk::projects::durable_log {

TEST(MetadataStoreTest, getFromEmptyStore) {
  InMemoryMetadataStore store;
  auto result = store.getConfig(0).semi().get();

  ASSERT_FALSE(result.has_value());
}

TEST(MetadataStoreTest, InitialVersionReturnedOnEmptyStoreIs0) {
  InMemoryMetadataStore store;
  ASSERT_EQ(store.getCurrentVersionId().semi().get(), 0);
}

TEST(MetadataStoreTest, addToEmptyStore) {
  // Adding first entry should always have version_id as 0;
  {
    InMemoryMetadataStore store;
    MetadataConfig config;
    ASSERT_THROW(store.compareAndAppendRange(4, config).semi().get(),
                 OptimisticConcurrencyException);
  }

  {
    InMemoryMetadataStore store;
    MetadataConfig config;
    ASSERT_THROW(store.compareAndAppendRange(10, config).semi().get(),
                 OptimisticConcurrencyException);
  }
}

TEST(MetadataStoreTest, addToAlreadyExistingStore) {
  InMemoryMetadataStore store;

  {
    MetadataConfig config;
    config.set_version_id(1);
    config.set_previous_version_id(0);
    config.set_start_index(500);
    config.set_end_index(1000);

    ASSERT_NO_THROW(store.compareAndAppendRange(0, config).semi().get());
  }

  // Add the next version.
  {
    MetadataConfig config;
    config.set_version_id(2);
    config.set_previous_version_id(1);
    config.set_start_index(1500);
    config.set_end_index(2000);

    ASSERT_NO_THROW(store.compareAndAppendRange(1, config).semi().get());
  }

  // Add the next version.
  {
    MetadataConfig config;
    config.set_version_id(3);
    config.set_previous_version_id(2);
    config.set_start_index(4500);
    config.set_end_index(5000);

    ASSERT_NO_THROW(store.compareAndAppendRange(2, config).semi().get());
  }
}

TEST(MetadataStoreTest, addToAlreadyExistingStoreSameConfig) {
  InMemoryMetadataStore store;

  {
    MetadataConfig config;
    config.set_version_id(1);
    config.set_previous_version_id(0);
    config.set_start_index(500);
    config.set_end_index(1000);

    ASSERT_NO_THROW(store.compareAndAppendRange(0, config).semi().get());
  }

  // Add the next version.
  {
    MetadataConfig config;
    config.set_version_id(2);
    config.set_previous_version_id(1);
    config.set_start_index(1500);
    config.set_end_index(2000);

    ASSERT_NO_THROW(store.compareAndAppendRange(1, config).semi().get());
  }

  // Add the next version.
  {
    MetadataConfig config;
    config.set_version_id(2);
    config.set_previous_version_id(1);
    config.set_start_index(1500);
    config.set_end_index(2000);

    ASSERT_THROW(store.compareAndAppendRange(1, config).semi().get(),
                 OptimisticConcurrencyException);
  }
}

TEST(MetadataStoreTest, addToAlreadyExistingStoreSkippingConfigs) {
  InMemoryMetadataStore store;

  {
    MetadataConfig config;
    config.set_version_id(1);
    config.set_previous_version_id(0);
    config.set_start_index(500);
    config.set_end_index(1000);

    ASSERT_NO_THROW(store.compareAndAppendRange(0, config).semi().get());
  }

  // Add the next version.
  {
    MetadataConfig config;
    config.set_version_id(2);
    config.set_previous_version_id(1);
    config.set_start_index(1500);
    config.set_end_index(2000);

    ASSERT_NO_THROW(store.compareAndAppendRange(1, config).semi().get());
  }

  // Add the skipped version.
  {
    MetadataConfig config;
    config.set_version_id(4);
    config.set_previous_version_id(2);
    config.set_start_index(1500);
    config.set_end_index(2000);

    ASSERT_THROW(store.compareAndAppendRange(2, config).semi().get(),
                 OptimisticConcurrencyException);
  }
}

TEST(MetadataStoreTest, getUsingLogId) {
  InMemoryMetadataStore store;

  {
    MetadataConfig config;
    config.set_version_id(1);
    config.set_previous_version_id(2);
    config.set_start_index(1500);
    config.set_end_index(2000);

    ASSERT_NO_THROW(store.compareAndAppendRange(0, config).semi().get());
  }

  {
    auto config = store.getConfigUsingLogId(1000).semi().get();
    ASSERT_FALSE(config.has_value());
  }

  {
    auto config = store.getConfigUsingLogId(1500).semi().get();
    ASSERT_TRUE(config.has_value());
  }

  {
    auto config = store.getConfigUsingLogId(1600).semi().get();
    ASSERT_TRUE(config.has_value());
  }

  // logId == 2000 should not exist.
  {
    auto config = store.getConfigUsingLogId(2000).semi().get();
    ASSERT_FALSE(config.has_value());
  }
}

TEST(MetadataStoreTest, getUsingLogIdWhenEmptyConfigsArePresent) {
  InMemoryMetadataStore store;

  {
    MetadataConfig config;
    config.set_version_id(1);
    config.set_previous_version_id(2);
    config.set_start_index(1500);
    config.set_end_index(2000);

    ASSERT_NO_THROW(store.compareAndAppendRange(0, config).semi().get());
  }

  {
    MetadataConfig config;
    config.set_version_id(2);
    config.set_previous_version_id(2);
    config.set_start_index(2000);
    config.set_end_index(2000);

    ASSERT_NO_THROW(store.compareAndAppendRange(1, config).semi().get());
  }

  {
    MetadataConfig config;
    config.set_version_id(3);
    config.set_previous_version_id(2);
    config.set_start_index(2000);
    config.set_end_index(2000);

    ASSERT_NO_THROW(store.compareAndAppendRange(2, config).semi().get());
  }

  {
    auto config = store.getConfigUsingLogId(2000).semi().get();
    ASSERT_FALSE(config.has_value());
  }

  {
    MetadataConfig config;
    config.set_version_id(4);
    config.set_previous_version_id(2);
    config.set_start_index(2000);
    config.set_end_index(2001);

    ASSERT_NO_THROW(store.compareAndAppendRange(3, config).semi().get());
  }

  // Now we should be able to fine 2000.
  {
    auto config = store.getConfigUsingLogId(2000).semi().get();
    ASSERT_TRUE(config.has_value());
  }
}

} // namespace rk::projects::durable_log
