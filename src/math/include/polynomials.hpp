//
// Created by Daniil Kazakov on 02.10.2025.
//

#pragma once

#include "types.h"

#include <vector>

// GF(2) многочлены (инверсия f) -- конечное поле из двух элементов 0, 1

struct Poly2 {
  std::vector<uint8_t> a; // uint8 0..255

  Poly2() = default;

  explicit Poly2(const int cap) { a.assign(cap + 1, 0); }
};

static int deg2(const Poly2 &p);

static Poly2 trim2(const Poly2 &p);

static Poly2 add2(const Poly2 &A, const Poly2 &B);

static Poly2 shl2_nonCirc(const Poly2 &A, int k);

static Poly2 mul2_nonCirc(const Poly2 &A, const Poly2 &B);

static void div2_poly(const Poly2 &A, const Poly2 &B, Poly2 &Q, Poly2 &R);

// инверсия f mod 2 (по модулю X^N + 1)
static bool invertMod2(const ntru::Poly &f, ntru::Poly &inv2_out);

// поднятие Хензеля до mod Q
static ntru::Poly henselLiftToQ(const ntru::Poly &f, const ntru::Poly &inv2);
