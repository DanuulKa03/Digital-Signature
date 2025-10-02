//
// Created by Daniil Kazakov on 02.10.2025.
//

#pragma once

#include<math/common.hpp>

inline int modQ(long long x) {
  long long q = G_Q;
  x %= q;
  if (x < 0) x += q;
  return static_cast<int>(x);
}

inline int center(int a) {
  int q = G_Q;
  int v = a % q;
  if (v < 0) v += q;
  if (v > q / 2) v -= q;
  return v;
}

static Poly zeroPoly() { return Poly(G_N, 0); }

static Poly subMod(const Poly &A, const Poly &B) {
  Poly R(G_N, 0);
  for (int i = 0; i < G_N; ++i) R[i] = modQ(static_cast<long long>(A[i]) - B[i]);
  return R;
}

static Poly mulModQ(const Poly &A, const Poly &B) {
  PolyLL acc(G_N, 0);
  for (int ii = 0; ii < G_N; ++ii)
    if (A[ii]) {
      for (int jj = 0; jj < G_N; ++jj)
        if (B[jj]) {
          int k = ii + jj;
          if (k >= G_N) k -= G_N;
          acc[k] += static_cast<long long>(A[ii]) * B[jj];
        }
    }
  Poly R(G_N, 0);
  for (int i = 0; i < G_N; ++i) R[i] = modQ(acc[i]);
  return R;
}

// умножение по модулю 2^t (для Хензеля)
static Poly mulModPow2(const Poly &A, const Poly &B, int M) {
  std::vector<long long> acc(G_N, 0);
  long long mask = static_cast<long long>(M) - 1;
  for (int ii = 0; ii < G_N; ++ii)
    if ((A[ii] & mask) != 0) {
      for (int jj = 0; jj < G_N; ++jj)
        if ((B[jj] & mask) != 0) {
          int k = ii + jj;
          if (k >= G_N) k -= G_N;
          acc[k] += static_cast<long long>(A[ii]) * B[jj];
        }
    }
  Poly R(G_N, 0);
  for (int i = 0; i < G_N; ++i) R[i] = static_cast<int>(acc[i] & mask);
  return R;
}
