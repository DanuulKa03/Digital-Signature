#include "hash.h"

namespace ntru {

    EHash H_e_small(const Poly& z_modq, const std::vector<uint8_t>& msg) {
        uint64_t s = 0x243F6A8885A308D3ull;

        auto mix_u32 = [&](uint32_t v) { 
            s ^= static_cast<uint64_t>(v) * 0x9E3779B1u;
            s = splitmix64(s); 
            };

        auto mix_u16 = [&](uint16_t v) { 
            s ^= static_cast<uint64_t>(v) * 0x85EBCA6Bu;
            s = splitmix64(s); 
            };

        auto mix_u8 = [&](uint8_t  v) { 
            s ^= static_cast<uint64_t>(v) * 0xC2B2AE35u;
            s = splitmix64(s); 
            };

        mix_u32(0x48655F65u); // "He_e"
        mix_u32(static_cast<uint32_t>(G_N));
        mix_u32(static_cast<uint32_t>(G_Q));
        mix_u32(static_cast<uint32_t>(G_ALPHA));

        for (int i = 0; i < G_N; ++i) { 
            uint16_t v = static_cast<uint16_t>(z_modq[i]);
            mix_u16(v); 
        }

        for (uint8_t b : msg) mix_u8(b);

        auto next32 = [&]()->uint32_t { 
            return static_cast<uint32_t>(splitmix64(s) & 0xFFFFFFFFu);
            };

        const uint32_t M = static_cast<uint32_t>(2 * G_ALPHA + 1);
        const uint32_t L = (uint32_t)((65536u / M) * M);

        Poly e_small(G_N, 0);
        for (int i = 0; i < G_N; ++i) {
            uint32_t t; 

            do { 
                t = (next32() & 0xFFFFu); 
            } 

            while (t >= L);
            uint32_t u = t % M;
            e_small[i] = static_cast<int>(u) - G_ALPHA;
        }
        Poly e_mod(G_N, 0);
        for (int i = 0; i < G_N; ++i) { 
            int m = e_small[i] % G_Q; 

            if (m < 0) {
                m += G_Q;
            }
            e_mod[i] = m; 
        }
        return { e_small, e_mod };
    }

} // namespace ntru








