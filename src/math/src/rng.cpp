#include "rng.h"
#include <cmath>

namespace ntru {

    int sample_gauss_int(SimpleRng& rng, double sigma) {
        double s = 0.0;

        for (int i = 0; i < 12; ++i) { 
            uint32_t r = rng(); 
            double u = (r / 4294967296.0); 
            s += u; 
        }

        double z = (s - 6.0) * sigma;
        long long y = llround(z);

        if (y > (1ll << 31) - 1) {
            y = (1ll << 31) - 1;
        }

        if (y < -(1ll << 31)) {
            y = -(1ll << 31);
        }
        return static_cast<int>(y);
    }

} // namespace ntru
