//
// Created by Rahul  Kushwaha on 1/5/23.
//

#include <gtest/gtest.h>

#include "log/impl/InMemoryMetadataStore.h"
#include "log/impl/PersistentMetadataStore.h"
#include "wor/WORFactory.h"

namespace rk::projects::durable_log {

namespace {

enum class MetadataStoreType {
  InMemory,
  InMemoryWor,
  PaxosWor,
};

std::vector<ReplicaConfig> getReplicaConfigs() {
  std::vector<ReplicaConfig> replicaConfig;

  for (int i = 0; i < 5; i++) {
    ReplicaConfig config{};
    config.set_id(fmt::format("REPLICA_ID_{}", i));
    config.mutable_ip_address()->set_host("127.0.0.1");
    config.mutable_ip_address()->set_port(10002);
    config.set_local(true);

    replicaConfig.emplace_back(std::move(config));
  }

  return replicaConfig;
}

} // namespace

class MetadataStoreTests : public testing::TestWithParam<MetadataStoreType> {
protected:
  static std::unique_ptr<MetadataStore> getStore() {
    MetadataConfig config{};
    config.set_version_id(0);
    config.set_previous_version_id(-1);
    config.set_start_index(0);
    config.set_end_index(0);

    auto replicaConfigs = getReplicaConfigs();
    config.mutable_replica_set_config()->Add(replicaConfigs.begin(),
                                             replicaConfigs.end());

    auto metadataStoreType = GetParam();
    switch (metadataStoreType) {
    case MetadataStoreType::InMemory: {
      return std::make_unique<InMemoryMetadataStore>();
    }

    case MetadataStoreType::InMemoryWor: {
      auto chain = wor::makeChainUsingInMemoryWor();
      auto inMemoryMetadataStore = std::make_shared<InMemoryMetadataStore>();
      return std::make_unique<PersistentMetadataStore>(
          config, std::move(inMemoryMetadataStore), nullptr);
    }

    case MetadataStoreType::PaxosWor: {
      auto chain = wor::makeChainUsingPaxosWor();
      auto inMemoryMetadataStore = std::make_shared<InMemoryMetadataStore>();
      return std::make_unique<PersistentMetadataStore>(
          config, std::move(inMemoryMetadataStore), nullptr);
    }
    default:
      throw std::runtime_error{"unknown metadata store type"};
    }
  }
};

TEST_P(MetadataStoreTests, getVersionId0) {
  auto store = getStore();
  auto result = store->getConfig(0).semi().get();
  ASSERT_EQ(MetadataConfig{}.DebugString(), result.value().DebugString());
}

TEST_P(MetadataStoreTests, getFromEmptyStore) {
  auto store = getStore();
  auto result = store->getConfig(1).semi().get();

  ASSERT_FALSE(result.has_value());
}

TEST_P(MetadataStoreTests, InitialVersionReturnedOnEmptyStoreIs0) {
  auto store = getStore();
  ASSERT_EQ(store->getCurrentVersionId().semi().get(), 0);
}

TEST_P(MetadataStoreTests, addToEmptyStore) {
  // Adding first entry should always have version_id as 0;
  {
    auto store = getStore();
    auto replicaConfigs = getReplicaConfigs();
    MetadataConfig config;
    config.set_previous_version_id(4);
    config.mutable_replica_set_config()->Add(replicaConfigs.begin(),
                                             replicaConfigs.end());
    ASSERT_THROW(store->compareAndAppendRange(config).semi().get(),
                 OptimisticConcurrencyException);
  }

  {
    auto store = getStore();
    MetadataConfig config;
    config.set_previous_version_id(10);
    ASSERT_THROW(store->compareAndAppendRange(config).semi().get(),
                 OptimisticConcurrencyException);
  }
}

TEST_P(MetadataStoreTests, addToAlreadyExistingStore) {
  auto store = getStore();

  {
    MetadataConfig config;
    config.set_version_id(1);
    config.set_previous_version_id(0);
    config.set_start_index(500);
    config.set_end_index(1000);
    auto replicaConfigs = getReplicaConfigs();
    config.mutable_replica_set_config()->Add(replicaConfigs.begin(),
                                             replicaConfigs.end());

    ASSERT_NO_THROW(store->compareAndAppendRange(config).semi().get());
  }

  // Add the next version.
  {
    MetadataConfig config;
    config.set_version_id(2);
    config.set_previous_version_id(1);
    config.set_start_index(1500);
    config.set_end_index(2000);
    auto replicaConfigs = getReplicaConfigs();
    config.mutable_replica_set_config()->Add(replicaConfigs.begin(),
                                             replicaConfigs.end());

    ASSERT_NO_THROW(store->compareAndAppendRange(config).semi().get());
  }

  // Add the next version.
  {
    MetadataConfig config;
    config.set_version_id(3);
    config.set_previous_version_id(2);
    config.set_start_index(4500);
    config.set_end_index(5000);
    auto replicaConfigs = getReplicaConfigs();
    config.mutable_replica_set_config()->Add(replicaConfigs.begin(),
                                             replicaConfigs.end());

    ASSERT_NO_THROW(store->compareAndAppendRange(config).semi().get());
  }
}

TEST_P(MetadataStoreTests, addToAlreadyExistingStoreSameConfig) {
  auto store = getStore();

  {
    MetadataConfig config;
    config.set_version_id(1);
    config.set_previous_version_id(0);
    config.set_start_index(500);
    config.set_end_index(1000);
    auto replicaConfigs = getReplicaConfigs();
    config.mutable_replica_set_config()->Add(replicaConfigs.begin(),
                                             replicaConfigs.end());

    ASSERT_NO_THROW(store->compareAndAppendRange(config).semi().get());
  }

  // Add the next version.
  {
    MetadataConfig config;
    config.set_version_id(2);
    config.set_previous_version_id(1);
    config.set_start_index(1500);
    config.set_end_index(2000);
    auto replicaConfigs = getReplicaConfigs();
    config.mutable_replica_set_config()->Add(replicaConfigs.begin(),
                                             replicaConfigs.end());

    ASSERT_NO_THROW(store->compareAndAppendRange(config).semi().get());
  }

  // Add the next version.
  {
    MetadataConfig config;
    config.set_version_id(2);
    config.set_previous_version_id(1);
    config.set_start_index(1500);
    config.set_end_index(2000);
    auto replicaConfigs = getReplicaConfigs();
    config.mutable_replica_set_config()->Add(replicaConfigs.begin(),
                                             replicaConfigs.end());

    ASSERT_THROW(store->compareAndAppendRange(config).semi().get(),
                 OptimisticConcurrencyException);
  }
}

TEST_P(MetadataStoreTests, addToAlreadyExistingStoreSkippingConfigs) {
  auto store = getStore();

  {
    MetadataConfig config;
    config.set_version_id(1);
    config.set_previous_version_id(0);
    config.set_start_index(500);
    config.set_end_index(1000);
    auto replicaConfigs = getReplicaConfigs();
    config.mutable_replica_set_config()->Add(replicaConfigs.begin(),
                                             replicaConfigs.end());

    ASSERT_NO_THROW(store->compareAndAppendRange(config).semi().get());
  }

  // Add the next version.
  {
    MetadataConfig config;
    config.set_version_id(2);
    config.set_previous_version_id(1);
    config.set_start_index(1500);
    config.set_end_index(2000);
    auto replicaConfigs = getReplicaConfigs();
    config.mutable_replica_set_config()->Add(replicaConfigs.begin(),
                                             replicaConfigs.end());

    ASSERT_NO_THROW(store->compareAndAppendRange(config).semi().get());
  }

  // Add the skipped version.
  {
    MetadataConfig config;
    config.set_version_id(4);
    config.set_previous_version_id(2);
    config.set_start_index(1500);
    config.set_end_index(2000);
    auto replicaConfigs = getReplicaConfigs();
    config.mutable_replica_set_config()->Add(replicaConfigs.begin(),
                                             replicaConfigs.end());

    ASSERT_THROW(store->compareAndAppendRange(config).semi().get(),
                 OptimisticConcurrencyException);
  }
}

TEST_P(MetadataStoreTests, getUsingLogId) {
  auto store = getStore();

  {
    MetadataConfig config;
    config.set_version_id(1);
    config.set_previous_version_id(0);
    config.set_start_index(1500);
    config.set_end_index(2000);
    auto replicaConfigs = getReplicaConfigs();
    config.mutable_replica_set_config()->Add(replicaConfigs.begin(),
                                             replicaConfigs.end());

    ASSERT_NO_THROW(store->compareAndAppendRange(config).semi().get());
  }

  {
    auto config = store->getConfigUsingLogId(1000).semi().get();
    ASSERT_FALSE(config.has_value());
  }

  {
    auto config = store->getConfigUsingLogId(1500).semi().get();
    ASSERT_TRUE(config.has_value());
  }

  {
    auto config = store->getConfigUsingLogId(1600).semi().get();
    ASSERT_TRUE(config.has_value());
  }

  // logId == 2000 should not exist.
  {
    auto config = store->getConfigUsingLogId(2000).semi().get();
    ASSERT_FALSE(config.has_value());
  }
}

TEST_P(MetadataStoreTests, getUsingLogIdWhenEmptyConfigsArePresent) {
  auto store = getStore();

  {
    MetadataConfig config;
    config.set_version_id(1);
    config.set_previous_version_id(0);
    config.set_start_index(1500);
    config.set_end_index(2000);
    auto replicaConfigs = getReplicaConfigs();
    config.mutable_replica_set_config()->Add(replicaConfigs.begin(),
                                             replicaConfigs.end());

    ASSERT_NO_THROW(store->compareAndAppendRange(config).semi().get());
  }

  {
    MetadataConfig config;
    config.set_version_id(2);
    config.set_previous_version_id(1);
    config.set_start_index(2000);
    config.set_end_index(2000);
    auto replicaConfigs = getReplicaConfigs();
    config.mutable_replica_set_config()->Add(replicaConfigs.begin(),
                                             replicaConfigs.end());

    ASSERT_NO_THROW(store->compareAndAppendRange(config).semi().get());
  }

  {
    MetadataConfig config;
    config.set_version_id(3);
    config.set_previous_version_id(2);
    config.set_start_index(2000);
    config.set_end_index(2000);
    auto replicaConfigs = getReplicaConfigs();
    config.mutable_replica_set_config()->Add(replicaConfigs.begin(),
                                             replicaConfigs.end());

    ASSERT_NO_THROW(store->compareAndAppendRange(config).semi().get());
  }

  {
    auto config = store->getConfigUsingLogId(2000).semi().get();
    ASSERT_FALSE(config.has_value());
  }

  {
    MetadataConfig config;
    config.set_version_id(4);
    config.set_previous_version_id(3);
    config.set_start_index(2000);
    config.set_end_index(2001);
    auto replicaConfigs = getReplicaConfigs();
    config.mutable_replica_set_config()->Add(replicaConfigs.begin(),
                                             replicaConfigs.end());

    ASSERT_NO_THROW(store->compareAndAppendRange(config).semi().get());
  }

  // Now we should be able to fine 2000.
  {
    auto config = store->getConfigUsingLogId(2000).semi().get();
    ASSERT_TRUE(config.has_value());
  }
}

INSTANTIATE_TEST_SUITE_P(MetadataStoreParameterizedTests, MetadataStoreTests,
                         testing::Values(MetadataStoreType::InMemory,
                                         MetadataStoreType::InMemoryWor,
                                         MetadataStoreType::PaxosWor));

} // namespace rk::projects::durable_log
