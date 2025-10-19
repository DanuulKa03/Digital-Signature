#pragma once
#include "../rng.h"
#include "../types.h"

namespace ntru {

  // �����
  extern Poly G_F, G_G, G_H;

  // ��������� ���������� ��������
  void genTernary(Poly &a, SimpleRng &rng);

  // keygen: F,G -> H = G * F^{-1} (mod q)
  [[nodiscard]] boolkeygen();

} // namespace ntru
