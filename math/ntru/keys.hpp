//
// Created by Daniil Kazakov on 02.10.2025.
//

#pragma once

#include <algorithm>
#include <random>
#include <vector>
#include <__ranges/iota_view.h>

#include "math/common.hpp"

static Poly G_Fkey, G_Gkey, G_Hpub;

static void genTernary(Poly &a) {
  a.assign(G_N, 0);
  std::vector<int> idx(G_N);
  std::ranges::views::iota(idx.begin(), idx.end(), 0);
  std::random_device rd;
  std::mt19937 rng(rd());
  shuffle(idx.begin(), idx.end(), rng);
  int plus = G_D / 2, minus = G_D - plus;
  if (plus == minus) {
    plus--;
    minus++;
  }
  for (int i = 0; i < plus; ++i) a[idx[i]] = 1;
  for (int i = plus; i < plus + minus; ++i) a[idx[i]] = G_Q - 1; // -1 mod Q
}

static bool keygen() {
  for (int tries = 0; tries < 100; ++tries) {
    genTernary(G_Fkey);
    genTernary(G_Gkey);
    Poly inv2(G_N, 0);
    if (!invertMod2(G_Fkey, inv2)) continue;
    Poly Finv = henselLiftToQ(G_Fkey, inv2);
    G_Hpub = mulModQ(Finv, G_Gkey);
    return true;
  }
  return false;
}
