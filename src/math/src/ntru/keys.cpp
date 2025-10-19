#include "math/keys.h"
#include <numeric>
#include "math/arithmetic.h"
#include "math/params.h"

namespace ntru {

  Poly G_F, G_G, G_H;

  void genTernary(Poly &a, SimpleRng &rng) {
    a.assign(G_N, 0);
    std::vector<int> idx(G_N);
    std::iota(idx.begin(), idx.end(), 0);

    for (int i = G_N - 1; i > 0; --i) {
      uint32_t r = rng() % (i + 1);
      std::swap(idx[i], idx[r]);
    }

    int plus = G_D / 2, minus = G_D - plus;
    if (plus == minus) {
      ++minus;
      --plus;
    }

    for (int i = 0; i < plus; ++i) {
      a[idx[i]] = 1;
    }

    for (int i = plus; i < plus + minus; ++i) {
      a[idx[i]] = G_Q - 1; // -1 mod q
    }
  }

  bool keygen() {
    SimpleRng rng;
    for (int tries = 0; tries < 2000; ++tries) {
      genTernary(G_F, rng);
      genTernary(G_G, rng);
      Poly inv2(G_N, 0);

      if (!invertMod2(G_F, inv2)) {
        continue;
      }

      Poly Finv = henselLiftToQ(G_F, inv2);
      G_H = mulModQ(Finv, G_G);
      return true;
    }
    return false;
  }

} // namespace ntru
