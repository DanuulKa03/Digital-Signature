//
// Created by Daniil Kazakov on 04.10.2025.
//

#include "../include/arithmetic.hpp"
#include "../include/polynomials.hpp"

int deg2(const Poly2 &p) {
  for (int i = static_cast<int>(p.a.size()) - 1; i >= 0; --i) if (p.a[i]) return i;
  return -1;
}

Poly2 trim2(const Poly2 &p) {
  const int d = deg2(p);
  Poly2 r(std::ranges::max(d, 0));
  for (int i = 0; i <= d; ++i) r.a[i] = p.a[i];
  return r;
}

Poly2 add2(const Poly2 &A, const Poly2 &B) {
  const int m = static_cast<int>(std::ranges::max(A.a.size(), B.a.size()));
  Poly2 R(m - 1);
  R.a.assign(m, 0);
  for (int i = 0; i < m; ++i) {
    const uint8_t va = (i < static_cast<int>(A.a.size()) ? A.a[i] : 0);
    const uint8_t vb = (i < static_cast<int>(B.a.size()) ? B.a[i] : 0);
    R.a[i] = va ^ vb;
  }
  return trim2(R);
}

Poly2 shl2_nonCirc(const Poly2 &A, int k) {
  if (k <= 0) return A;
  Poly2 R(static_cast<int>(A.a.size()) - 1 + k);
  R.a.assign(A.a.size() + k, 0);
  for (int i = 0; i < static_cast<int>(A.a.size()); ++i) if (A.a[i]) R.a[i + k] ^= 1;
  return R;
}

Poly2 mul2_nonCirc(const Poly2 &A, const Poly2 &B) {
  Poly2 R(static_cast<int>(A.a.size()) + static_cast<int>(B.a.size()));
  R.a.assign(A.a.size() + B.a.size() + 1, 0);
  for (int i = 0; i < static_cast<int>(A.a.size()); ++i)
    if (A.a[i])
      for (int j = 0; j < static_cast<int>(B.a.size()); ++j)
        if (B.a[j]) {
          if (i + j >= static_cast<int>(R.a.size())) R.a.resize(i + j + 1, 0);
          R.a[i + j] ^= 1;
        }
  return trim2(R);
}

void div2_poly(const Poly2 &A, const Poly2 &B, Poly2 &Q, Poly2 &R) {
  Poly2 a = A;
  Q.a.assign(std::ranges::max(1, static_cast<int>(A.a.size())), 0);
  const int dB = deg2(B);
  if (dB < 0) {
    R = a;
    return;
  }
  while (true) {
    const int dA = deg2(a);
    if (dA < dB || dA < 0) break;
    const int s = dA - dB;
    Poly2 S(dA);
    S.a.assign(dA + 1, 0);
    S.a[s] = 1;
    Q = add2(Q, S);
    Poly2 SB = shl2_nonCirc(B, s);
    a = add2(a, SB);
  }
  R = a;
}

bool invertMod2(const Poly &f, Poly &inv2_out) {
  Poly2 F(G_N);
  F.a.assign(G_N + 1, 0);
  for (int i = 0; i < G_N; ++i) F.a[i] = (uint8_t)(f[i] & 1);
  Poly2 P(G_N);
  P.a.assign(G_N + 1, 0);
  P.a[0] = 1;
  P.a[G_N] = 1;

  Poly2 a = P, b = F, ua, va, ub, vb;
  ua.a = {1};
  va.a = {0};
  ub.a = {0};
  vb.a = {1};
  while (true) {
    if (deg2(b) < 0) {
      if (deg2(a) != 0 || a.a[0] != 1) return false;
      Poly2 Q, R;
      div2_poly(va, P, Q, R);
      inv2_out.assign(G_N, 0);
      for (int i = 0; i < G_N; ++i) inv2_out[i] = (i < static_cast<int>(R.a.size()) ? (R.a[i] & 1) : 0);
      return true;
    }
    Poly2 Q, R;
    div2_poly(a, b, Q, R);
    Poly2 Qu = mul2_nonCirc(Q, ub), Qv = mul2_nonCirc(Q, vb);
    Poly2 n_ub = add2(ua, Qu), n_vb = add2(va, Qv);
    ua = ub;
    va = vb;
    ub = n_ub;
    vb = n_vb;
    a = b;
    b = R;
  }
}

Poly henselLiftToQ(const Poly &f, const Poly &inv2) {
  Poly inv = inv2;
  for (int i = 0; i < G_N; ++i) inv[i] &= 1;
  int M = 2;
  while (M < G_Q) {
    Poly t = mulModPow2(inv, f, M);
    const int nextM = M << 1;
    const long long mask = static_cast<long long>(nextM) - 1;
    Poly corr(G_N, 0);
    for (int i = 0; i < G_N; ++i) {
      int ti = t[i] & (M - 1);
      int v = (2 - ti) & (nextM - 1);
      corr[i] = v;
    }
    inv = mulModPow2(inv, corr, nextM);
    for (int i = 0; i < G_N; ++i) inv[i] = static_cast<int>(inv[i] & mask);
    M = nextM;
  }
  Poly res(G_N, 0);
  for (int i = 0; i < G_N; ++i) res[i] = inv[i] & (G_Q - 1);
  return res;
}
