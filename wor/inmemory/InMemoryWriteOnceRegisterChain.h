//
// Created by Rahul  Kushwaha on 6/7/23.
//

#pragma once
#include "wor/WriteOnceRegisterChainImpl.h"
#include "wor/inmemory/InMemoryWriteOnceRegister.h"

#include <atomic>
#include <map>
#include <mutex>

namespace rk::projects::wor {

class InMemoryWriteOnceRegisterChain : public WriteOnceRegisterChainImpl {
public:
  explicit InMemoryWriteOnceRegisterChain(
      std::function<std::shared_ptr<WriteOnceRegister>()> registerFactory)
      : WriteOnceRegisterChainImpl(std::move(registerFactory)) {}
};

} // namespace rk::projects::wor
