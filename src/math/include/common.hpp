//
// Created by Daniil Kazakov on 02.10.2025.
//

#pragma once

#include <vector>

using Poly = std::vector<int>;
using PolyLL = std::vector<long long>;

struct EHash {
  Poly e_small;
  Poly e_mod;
};

struct Signature {
  Poly x1, x2, e;
};

static int G_N = 0; // степень кольца
static int G_Q = 0; // модуль по коэффициентам
static int G_D = 0; // вес тернарных ключей
static double G_NU = 0; // коэффициент в норме NTRUSign_once
static int G_NORM_BOUND = 0; // порог нормы для s,t
static double G_ETA = 0; // коэффициент для bound подписи
static int G_ALPHA = 0; // альфа (размер малых e)
static int G_SIGMA = 0; // стд. отклонение Гаусса
static double G_MACC = 0; // нормировочный коэффициент для rejection
static int G_MAX_SIGN_ATT = 1000; // потолок попыток маскирования
