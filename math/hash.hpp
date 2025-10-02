//
// Created by Daniil Kazakov on 02.10.2025.
//

#pragma once
#include <cstdint>
#include <vector>

#include "math/common.hpp"

static inline EHash H_e_small(const Poly &z_modq, const std::vector<uint8_t> &msg) {
  std::vector < uint8_t > buf;
  buf.reserve(msg.size() + 2u * (size_t) G_N);
  buf.insert(buf.end(), msg.begin(), msg.end());
  for (int i = 0; i < G_N; ++i) {
    uint16_t v = static_cast<uint16_t>(z_modq[i]);
    buf.push_back(static_cast<uint8_t>(v & 0xFF));
    buf.push_back(static_cast<uint8_t>(v >> 8));
  }
  uint32_t s1 = 0x243F6A88u, s2 = 0x85A308D3u;

  Poly e_small(G_N, 0);
  for (size_t i = 0; i < buf.size(); ++i) {
    s1 = (s1 + buf[i] + (s2 << 5) + (s2 >> 2)) * 2654435761u;
    s2 ^= (s1 << 7) | (s1 >> 25);
    int pos = (int) (s1 % (uint32_t) G_N);
    int u = (int) ((s2 & 0x7FFFFFFF) % (2 * G_ALPHA + 1));
    int val = u - G_ALPHA;
    int x = e_small[pos] + val;
    if (x > G_ALPHA) x = G_ALPHA;
    if (x < -G_ALPHA) x = -G_ALPHA;
    e_small[pos] = x;
  }
  Poly e_mod(G_N, 0);
  for (int i = 0; i < G_N; ++i) {
    int m = e_small[i] % G_Q;
    if (m < 0) m += G_Q;
    e_mod[i] = m;
  }
  return {e_small, e_mod};
}
