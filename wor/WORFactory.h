//
// Created by Rahul  Kushwaha on 7/8/23.
//
#pragma once

#include "log/proto/MetadataConfig.pb.h"
#include "wor/WriteOnceRegisterChainImpl.h"
#include "wor/inmemory/InMemoryWriteOnceRegister.h"

namespace rk::projects::wor {

std::unique_ptr<WriteOnceRegister>
makePaxosWor(WorId worId, const durable_log::MetadataConfig &metadataConfig);
std::unique_ptr<WriteOnceRegisterChain> makeChainUsingInMemoryWor();
std::unique_ptr<WriteOnceRegisterChain> makeChainUsingPaxosWor();

} // namespace rk::projects::wor