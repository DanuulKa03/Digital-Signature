#include "sign.h"
#include <cmath>
#include "arithmetic.h"
#include "hash.h"
#include "keys.h"
#include "params.h"
#include "rng.h"

#include "ntru/keys.h"

namespace ntru {

  bool NTRUSign_once(const Poly &m, Poly &s_out) {

    std::vector<int> fI(G_N, 0), gI(G_N, 0), mI(G_N, 0);
    for (int i = 0; i < G_N; ++i) {
      fI[i] = center(G_F[i]);
      gI[i] = center(G_G[i]);
      mI[i] = center(m[i]);
    }

    std::vector<long long> xA(G_N, 0), yA(G_N, 0);
    for (int i = 0; i < G_N; ++i)
      if (mI[i]) {
        for (int j = 0; j < G_N; ++j) {
          int k = i + j;
          if (k >= G_N) {
            k -= G_N;
          }
          xA[k] -= static_cast<long long>(mI[i]) * gI[j];
          yA[k] += static_cast<long long>(mI[i]) * fI[j];
        }
      }
    std::vector<int> kx(G_N, 0), ky(G_N, 0);
    for (int i = 0; i < G_N; ++i) {
      kx[i] = static_cast<int>(llround(static_cast<long double>(xA[i]) / static_cast<long double>(G_Q)));
      ky[i] = static_cast<int>(llround(static_cast<long double>(yA[i]) / static_cast<long double>(G_Q)));
    }
    std::vector<long long> sA(G_N, 0);
    for (int i = 0; i < G_N; ++i) {
      if (kx[i])
        for (int j = 0; j < G_N; ++j) {
          int k = i + j;
          if (k >= G_N) {
            k -= G_N;
          }
          sA[k] += static_cast<long long>(kx[i]) * fI[j];
        }
      if (ky[i])
        for (int j = 0; j < G_N; ++j) {
          int k = i + j;
          if (k >= G_N) {
            k -= G_N;
          }
          sA[k] += static_cast<long long>(ky[i]) * gI[j];
        }
    }
    s_out.assign(G_N, 0);

    for (int i = 0; i < G_N; ++i) {
      s_out[i] = static_cast<int>(sA[i]);
    }

    Poly sMod(G_N, 0);
    for (int i = 0; i < G_N; ++i) {
      sMod[i] = modQ(s_out[i]);
    }

    Poly sh = mulModQ(sMod, G_H);
    std::vector<int> tI(G_N, 0);
    for (int i = 0; i < G_N; ++i) {
      tI[i] = center(modQ(static_cast<long long>(sh[i]) - m[i]));
    }

    long double s2 = 0, t2 = 0;

    for (int i = 0; i < G_N; ++i) {
      s2 += static_cast<long double>(s_out[i]) * s_out[i];
      t2 += static_cast<long double>(tI[i]) * tI[i];
    }

    const long double norm2 = s2 + (G_NU * G_NU) * t2;

    return (norm2 <= static_cast<long double>(G_NORM_BOUND) * static_cast<long double>(G_NORM_BOUND));
  }

  bool sign_strict(const std::vector<uint8_t> &msg, Signature &sig) {
    SimpleRng rng;
    for (int tries = 0; tries < G_MAX_SIGN_ATT; ++tries) {
      // 1) y ~ D_sigma
      Poly y1I(G_N, 0), y2I(G_N, 0);
      for (int i = 0; i < G_N; ++i) {
        y1I[i] = sample_gauss_int(rng, G_SIGMA);
        y2I[i] = sample_gauss_int(rng, G_SIGMA);
      }
      Poly y1(G_N, 0), y2(G_N, 0);
      for (int i = 0; i < G_N; ++i) {
        y1[i] = modQ(y1I[i]);
        y2[i] = modQ(y2I[i]);
      }

      // 2) e = H_e_small(z,msg), z = y2 - h*y1
      Poly hy1 = mulModQ(G_H, y1);
      Poly z = subMod(y2, hy1);
      EHash eh = H_e_small(z, msg);

      // 3) NTRUSign_once на m=e_mod -> s,t
      Poly sI;
      if (!NTRUSign_once(eh.e_mod, sI))
        continue;
      Poly sMod(G_N, 0);

      for (int i = 0; i < G_N; ++i) {
        sMod[i] = modQ(sI[i]);
      }

      Poly sh = mulModQ(sMod, G_H);
      std::vector<int> tI(G_N, 0);
      for (int i = 0; i < G_N; ++i) {
        tI[i] = center(modQ(static_cast<long long>(sh[i]) - eh.e_mod[i]));
      }

      // 4) (x1,x2) = (y1 - s,  y2 - t - e_small)
      Poly x1(G_N, 0), x2(G_N, 0);
      for (int i = 0; i < G_N; ++i) {
        int xi1 = y1I[i] - sI[i];
        int xi2 = y2I[i] - tI[i] - eh.e_small[i];
        x1[i] = modQ(xi1);
        x2[i] = modQ(xi2);
      }

      // 5) rejection
      long double sigma2 = static_cast<long double>(G_SIGMA) * static_cast<long double>(G_SIGMA);
      long double dot = 0.0L, v2 = 0.0L, xnorm2 = 0.0L;
      for (int i = 0; i < G_N; ++i) {
        int xv1 = center(x1[i]), xv2 = center(x2[i]);
        int vv1 = -sI[i], vv2 = -tI[i] - eh.e_small[i];
        dot += (long double) xv1 * vv1 + (long double) xv2 * vv2;
        v2 += (long double) vv1 * vv1 + (long double) vv2 * vv2;
        xnorm2 += (long double) xv1 * xv1 + (long double) xv2 * xv2;
      }
      long double exponent = (dot - 0.5L * v2) / sigma2;
      if (exponent > 700.0L)
        exponent = 700.0L;
      if (exponent < -700.0L)
        exponent = -700.0L;
      long double R = expl(exponent);
      long double p = R / G_MACC;
      if (p > 1.0L)
        p = 1.0L;
      if (!std::isfinite((double) p) || p != p)
        p = 0.0L;

      uint32_t r = rng();
      long double U = ((long double) r) / 4294967296.0L;
      if (U > p)
        continue;

      long double bound = (long double) G_ETA * (long double) G_SIGMA * sqrtl(2.0L * (long double) G_N);
      if (sqrtl(xnorm2) > bound)
        continue;

      sig.x1 = std::move(x1);
      sig.x2 = std::move(x2);
      sig.e = std::move(eh.e_mod);
      return true;
    }
    return false;
  }

} // namespace ntru
