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

  std::string generate() { return to_string(state_->gen()); }

 private:
  explicit UuidGenerator() : state_{std::make_unique<State>()} {}

  struct State {
    std::random_device rd;
    std::array<int, std::mt19937::state_size> seedData;
    std::mt19937 generator;
    uuids::uuid_random_generator gen;

    explicit State()
        : rd{},
          seedData{},
          generator{[this]() {
            std::generate(std::begin(seedData), std::end(seedData),
                          std::ref(rd));
            std::seed_seq seq(std::begin(seedData), std::end(seedData));
            return std::mt19937(seq);
          }()},
          gen{[this]() {
            return uuids::uuid_random_generator{generator};
          }()} {}
  };

  std::shared_ptr<State> state_;
};

}  // namespace rk::projects::utils