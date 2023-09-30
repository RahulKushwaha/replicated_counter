//
// Created by Rahul  Kushwaha on 7/8/23.
//
#pragma once

#include "log/proto/MetadataConfig.pb.h"
#include "wor/WriteOnceRegisterChainImpl.h"
#include "wor/inmemory/InMemoryWriteOnceRegister.h"
#include "wor/paxos/include/Registry.h"

namespace rk::projects::wor {

std::unique_ptr<WriteOnceRegister> makePaxosWor(
    WorId worId, const std::vector<durable_log::ReplicaConfig>& replicaConfigs,
    const std::shared_ptr<paxos::Registry>& registry);

std::unique_ptr<WriteOnceRegisterChain> makeChainUsingInMemoryWor();

std::unique_ptr<WriteOnceRegisterChain> makeChainUsingPaxosWor();

}  // namespace rk::projects::wor