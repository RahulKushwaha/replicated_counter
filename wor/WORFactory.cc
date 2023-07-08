//
// Created by Rahul  Kushwaha on 7/8/23.
//

#include "WORFactory.h"

namespace rk::projects::wor {

std::unique_ptr<WriteOnceRegisterChain> makeChainUsingInMemoryWor() {
  return std::make_unique<WriteOnceRegisterChainImpl>(
      std::move(std::make_shared<InMemoryWriteOnceRegister>));
}

} // namespace rk::projects::wor