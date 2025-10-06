#pragma once
#include <vector>

namespace ntru {

    using Poly = std::vector<int>;

    struct EHash {
        Poly e_small;
        Poly e_mod;
    };

    struct Signature {
        Poly x1, x2, e;
    };

} // namespace ntru
