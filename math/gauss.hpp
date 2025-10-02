//
// Created by Daniil Kazakov on 02.10.2025.
//

#pragma once

#include <random>

static int sample_gauss_int(std::mt19937 &rng, const double sigma) {
  std::normal_distribution<double> norm01(0.0, 1.0);
  const double x = norm01(rng) * sigma;
  long long y = llround(x);
  if (y > (1ll << 31) - 1) y = (1ll << 31) - 1;
  if (y < -(1ll << 31)) y = -(1ll << 31);
  return static_cast<int>(y);
}
