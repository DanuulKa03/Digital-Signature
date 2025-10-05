//
// Created by Daniil Kazakov on 02.10.2025.
//

#pragma once

#include "common.hpp"

static int modQ(long long x);

static int center(int a);

static Poly zeroPoly();

static Poly subMod(const Poly &A, const Poly &B);

static Poly mulModQ(const Poly &A, const Poly &B);

// умножение по модулю 2^t (для Хензеля)
static Poly mulModPow2(const Poly &A, const Poly &B, int M);
