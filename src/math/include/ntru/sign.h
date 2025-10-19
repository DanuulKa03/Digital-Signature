#pragma once
#include <cstdint>
#include <vector>
#include "types.h"

namespace ntru {

  // «ядро» NTRUSign для m = e_mod
  [[nodiscard]] boolNTRUSign_once(const Poly &m, Poly &s_out);

  // sealing-подпись (x1,x2,e)
  [[nodiscard]] boolsign_strict(const std::vector<uint8_t> &msg, Signature &sig);

} // namespace ntru
