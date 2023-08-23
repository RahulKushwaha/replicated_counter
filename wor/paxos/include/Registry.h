//
// Created by Rahul  Kushwaha on 8/19/23.
//
#pragma once
#include "wor/paxos/include/Acceptor.h"

namespace rk::projects::paxos {

class Registry {
public:
  virtual void registerAcceptor(std::string id,
                                std::shared_ptr<Acceptor> acceptor) = 0;

  virtual std::shared_ptr<Acceptor> getAcceptor(std::string id) = 0;

  virtual ~Registry() = default;
};

} // namespace rk::projects::paxos