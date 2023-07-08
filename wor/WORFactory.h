//
// Created by Rahul  Kushwaha on 7/8/23.
//
#pragma once

#include "wor/WriteOnceRegisterChainImpl.h"
#include "wor/inmemory/InMemoryWriteOnceRegister.h"

namespace rk::projects::wor {

std::unique_ptr<WriteOnceRegisterChain> makeChainUsingInMemoryWor();

} // namespace rk::projects::wor