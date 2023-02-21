//
// Created by Rahul  Kushwaha on 2/20/23.
//
#pragma once
#include <memory>

#include "../include/MetadataStore.h"
#include "../include/NanoLogStore.h"
#include "../include/VirtualLog.h"
#include "InMemoryMetadataStore.h"
#include "NanoLogStoreImpl.h"
#include "../include/Replica.h"
#include "ReplicaImpl.h"

namespace rk::projects::durable_log {

std::unique_ptr<VirtualLog> makeVirtualLog(std::string name);
std::unique_ptr<NanoLogStore> makeNanoLogStore();
std::unique_ptr<MetadataStore> makeMetadataStore();

std::unique_ptr<Replica>
makeReplica(std::string id,
            std::string name,
            std::shared_ptr<NanoLogStore> nanoLogStore,
            std::shared_ptr<MetadataStore> metadataStore);

std::unique_ptr<Sequencer>
makeSequencer(std::vector<std::shared_ptr<Replica>> replicaSet);

}
