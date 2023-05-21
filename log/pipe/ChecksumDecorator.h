//
// Created by Rahul  Kushwaha on 5/20/23.
//

#pragma once
#include "log/include/Common.h"
#include "log/pipe/proto/ChecksumProtectedLogEntry.pb.h"

namespace rk::projects::durable_log {

class ChecksumDecorator {
 public:
  std::string wrap(std::string payload);
  std::variant<ChecksumProtectedLogEntry, LogReadError>
  unwrap(std::string payload);
};

}