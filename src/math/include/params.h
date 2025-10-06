#pragma once
#include <cmath>

namespace ntru {

    // Глобальные параметры (учебный вариант)
    extern int    G_N;
    extern int    G_Q;
    extern int    G_D;
    extern int    G_NORM_BOUND;
    extern int    G_ALPHA;
    extern int    G_SIGMA;
    extern int    G_MAX_SIGN_ATT;

    extern double G_NU;
    extern double G_ETA;
    extern double G_MACC;

    // Установить параметры из приложения (без файлового IO)
    inline void set_params(int N, int Q, int D, double NU, int NORM_BOUND,
        double ETA, int ALPHA, int SIGMA, int MAX_SIGN_ATT = 1000)
    {
        G_N = N; G_Q = Q; G_D = D; G_NU = NU; G_NORM_BOUND = NORM_BOUND;
        G_ETA = ETA; G_ALPHA = ALPHA; G_SIGMA = SIGMA; G_MAX_SIGN_ATT = MAX_SIGN_ATT;
        G_MACC = std::exp(1.0 + 1.0 / (2.0 * (double)G_ALPHA * (double)G_ALPHA));
    }

} // namespace ntru
