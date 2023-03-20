//
// Created by Rahul  Kushwaha on 2/26/23.
//
#pragma once

#include <cstdint>
#include <cstddef>

#include <array>
#include "EnsembleNode.h"

namespace rk::projects::durable_log {

template<class APP, size_t SIZE = 5>
struct Ensemble {
  // Make sure size is an odd number.
  static_assert(SIZE % 2 == 1);

  static constexpr size_t QUORUM_SIZE = SIZE / 2 + 1;
  static constexpr size_t ENSEMBLE_SIZE = SIZE;

  std::array<EnsembleNode<APP>, SIZE> nodes_;
};

}