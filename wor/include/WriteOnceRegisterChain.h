//
// Created by Rahul  Kushwaha on 6/7/23.
//
#pragma once
#include "WriteOnceRegister.h"

namespace rk::projects::wor {

using WorId = std::int64_t;

class WriteOnceRegisterChain {
 public:
  virtual std::optional<WorId> append() = 0;
  virtual std::optional<std::shared_ptr<WriteOnceRegister>> get(WorId id) = 0;
  virtual std::optional<WorId> tail() = 0;
  virtual std::optional<WorId> front() = 0;
};

}