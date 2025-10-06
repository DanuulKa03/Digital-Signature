#pragma once
#include <vector>
#include <cstdint>
#include "types.h"

namespace ntru {

	// «ядро» NTRUSign для m = e_mod
	bool NTRUSign_once(const Poly& m, Poly& s_out);

	// sealing-подпись (x1,x2,e)
	bool sign_strict(const std::vector<uint8_t>& msg, Signature& sig);

} // namespace ntru
