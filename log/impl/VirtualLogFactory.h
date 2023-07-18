//
// Created by Rahul  Kushwaha on 2/20/23.
//
#pragma once
#include <memory>

#include "../include/MetadataStore.h"
#include "../include/NanoLogStore.h"
#include "../include/Registry.h"
#include "../include/Replica.h"
#include "../include/VirtualLog.h"
#include "InMemoryMetadataStore.h"
#include "NanoLogStoreImpl.h"
#include "ReplicaImpl.h"

namespace rk::projects::durable_log {

std::unique_ptr<VirtualLog> makeVirtualLog(std::string name);
std::unique_ptr<NanoLogStore> makeNanoLogStore();
std::unique_ptr<MetadataStore> makeMetadataStore();

std::unique_ptr<Replica>
makeReplica(std::string id, std::string name,
            std::shared_ptr<NanoLogStore> nanoLogStore,
            std::shared_ptr<MetadataStore> metadataStore);

std::unique_ptr<Replica>
makeRocksReplica(std::string id, std::string name,
                 std::shared_ptr<NanoLogStore> nanoLogStore,
                 std::shared_ptr<MetadataStore> metadataStore,
                 persistence::RocksDbFactory::RocksDbConfig rocksDbConfig);

std::unique_ptr<Sequencer>
makeSequencer(std::string sequencerId,
              std::vector<std::shared_ptr<Replica>> replicaSet);

std::unique_ptr<Registry> makeRegistry();

} // namespace rk::projects::durable_log
