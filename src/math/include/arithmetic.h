#include <span>
#pragma once
#include <cstdint>
#include <vector>
#include "math/params.h"
#include "types.h"

namespace ntru {

  // базовые операции
  int modQ(long long x) noexcept;
  int center(int a) noexcept;
  Poly zeroP() noexcept;

  // полиномиальная арифметика в R_q = Z_q[X]/(X^N-1)
  Poly subMod(std::span<const int> A, std::span<const int> B) noexcept;
  Poly mulModQ(std::span<const int> A, std::span<const int> B) noexcept;
  Poly mulModPow2(std::span<const int> A, std::span<const int> B, int M) noexcept;

  // GF(2) инверсия + Хензель
  struct Poly2 {
    std::vector<uint8_t> a;
    Poly2();
    Poly2(int cap);
  };
  [[nodiscard]] int deg2(const Poly2 &p);
  Poly2 trim2(const Poly2 &p);
  Poly2 add2(const Poly2 &A, const Poly2 &B);
  Poly2 shl2_nonCirc(const Poly2 &A, int k);
  Poly2 mul2_nonCirc(const Poly2 &A, const Poly2 &B);
  void div2_poly(const Poly2 &A, const Poly2 &B, Poly2 &Q, Poly2 &R);
  [[nodiscard]] bool invertMod2(const Poly &f, Poly &inv2_out);
  [[nodiscard]] Poly henselLiftToQ(const Poly &f, const Poly &inv2);

} // namespace ntru
