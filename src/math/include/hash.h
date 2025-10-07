#pragma once
#include <cstdint>
#include <vector>
#include "params.h"
#include "types.h"

namespace ntru {

    // splitmix64 helper
    inline uint64_t splitmix64(uint64_t& x) {
        x += 0x9E3779B97F4A7C15ull;
        uint64_t z = x;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        return z ^ (z >> 31);
    }

    // H_e_small: детерминированный поток (НЕ криптостойкий), unbiased
    EHash H_e_small(const Poly& z_modq, const std::vector<uint8_t>& msg);

} // namespace ntru
