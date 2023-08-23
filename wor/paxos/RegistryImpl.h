//
// Created by Rahul  Kushwaha on 8/19/23.
//
#pragma once
#include "wor/paxos/include/Registry.h"

namespace rk::projects::paxos {

class RegistryImpl : public Registry {
public:
  RegistryImpl() = default;

  void registerAcceptor(std::string id,
                        std::shared_ptr<Acceptor> acceptor) override {
    auto result = lookup_.emplace(std::move(id), std::move(acceptor));
    assert(result.second);
  }

  std::shared_ptr<Acceptor> getAcceptor(std::string id) override {
    auto itr = lookup_.find(id);
    assert(itr != lookup_.end());

    return itr->second;
  }

private:
  std::unordered_map<std::string, std::shared_ptr<Acceptor>> lookup_;
};

} // namespace rk::projects::paxos