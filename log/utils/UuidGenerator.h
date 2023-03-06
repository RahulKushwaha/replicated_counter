//
// Created by Rahul  Kushwaha on 3/5/23.
//

#pragma once

#include "uuid.h"

namespace rk::projects::utils {

class UuidGenerator final {
 public:
  static UuidGenerator instance() {
    static UuidGenerator uuidGenerator{};
    return uuidGenerator;
  }

  std::string generate() {
    return to_string(gen_());
  }

 private:
  explicit UuidGenerator() : gen_{[]() {
    std::random_device rd;
    auto seed_data = std::array<int, std::mt19937::state_size>{};
    std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::mt19937 generator(seq);
    uuids::uuid_random_generator gen{generator};
    return gen;
  }()} {}

  uuids::uuid_random_generator gen_;
};

}