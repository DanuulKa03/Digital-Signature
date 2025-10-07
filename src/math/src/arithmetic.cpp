#include "arithmetic.h"
#include <algorithm>

namespace ntru {

    int modQ(long long x) { 
        long long q = G_Q; 
        x %= q; 
        if (x < 0) {
            x += q;
        }
        return (int)x; 
    }
    int center(int a) { 
        int q = G_Q; 
        int v = a % q; 
        if (v < 0) {
            v += q;
        }
        if (v > q / 2) {
            v -= q;
        }
        return v; 
    }
    Poly zeroP() { 
        return Poly(G_N, 0); 
    }

    Poly subMod(const Poly& A, const Poly& B) {
        Poly R(G_N, 0);
        for (int i = 0; i < G_N; ++i) {
            R[i] = modQ((long long)A[i] - B[i]);
        }
        return R;
    }
    Poly mulModQ(const Poly& A, const Poly& B) {
        std::vector<long long> acc(G_N, 0);
        for (int i = 0; i < G_N; ++i) if (A[i]) {
            for (int j = 0; j < G_N; ++j) if (B[j]) {
                int k = i + j; if (k >= G_N) k -= G_N;
                acc[k] += (long long)A[i] * B[j];
            }
        }
        Poly R(G_N, 0);
        for (int i = 0; i < G_N; ++i) {
            R[i] = modQ(acc[i]);
        }
        return R;
    }
    Poly mulModPow2(const Poly& A, const Poly& B, int M) {
        long long mask = (long long)M - 1;
        std::vector<long long> acc(G_N, 0);
        for (int i = 0; i < G_N; ++i) if ((A[i] & mask) != 0) {
            for (int j = 0; j < G_N; ++j) if ((B[j] & mask) != 0) {
                int k = i + j; if (k >= G_N) k -= G_N;
                acc[k] += (long long)A[i] * B[j];
            }
        }
        Poly R(G_N, 0);
        for (int i = 0; i < G_N; ++i) {
            R[i] = (int)(acc[i] & mask);
        }
        return R;
    }

    // GF(2) + Хензель
    Poly2::Poly2() {} 
    Poly2::Poly2(int cap) { 
        a.assign(cap + 1, 0); 
    }

    int deg2(const Poly2& p) { 
        for (int i = (int)p.a.size() - 1; i >= 0; --i) {
            if (p.a[i]) {
                return i;
            }
        }  
        return -1; 
    }

    Poly2 trim2(const Poly2& p) { 
        int d = deg2(p); 
        Poly2 r(std::max(d, 0)); 
        for (int i = 0; i <= d; ++i) {
            r.a[i] = p.a[i];
        }
        return r; 
    }

    Poly2 add2(const Poly2& A, const Poly2& B) {
        int m = (int)std::max(A.a.size(), B.a.size()); 
        Poly2 R(m - 1); 
        R.a.assign(m, 0);
        for (int i = 0; i < m; ++i) { 
            uint8_t va = (i < (int)A.a.size() ? A.a[i] : 0), vb = (i < (int)B.a.size() ? B.a[i] : 0); 
            R.a[i] = va ^ vb; 
        }
        return trim2(R);
    }

    Poly2 shl2_nonCirc(const Poly2& A, int k) {
        if (k <= 0) 
            return A;
        Poly2 R((int)A.a.size() - 1 + k); R.a.assign(A.a.size() + k, 0);
        for (int i = 0; i < (int)A.a.size(); ++i) {
            if (A.a[i]) {
                R.a[i + k] ^= 1;
            }
        }
        return R;
    }


    Poly2 mul2_nonCirc(const Poly2& A, const Poly2& B) {
        Poly2 R((int)(A.a.size() + B.a.size())); 
        R.a.assign((int)(A.a.size() + B.a.size()) + 1, 0);
        for (int i = 0; i < (int)A.a.size(); ++i) if (A.a[i]) {
            for (int j = 0; j < (int)B.a.size(); ++j) if (B.a[j]) {
                int pos = i + j; if (pos >= (int)R.a.size()) R.a.resize(pos + 1, 0);
                R.a[pos] ^= 1;
            }
        }
        return trim2(R);
    }
    void div2_poly(const Poly2& A, const Poly2& B, Poly2& Q, Poly2& R) {
        Poly2 a = A; Q.a.assign(std::max(1, (int)A.a.size()), 0);
        int dB = deg2(B); if (dB < 0) { R = a; return; }
        while (true) {
            int dA = deg2(a); if (dA < dB || dA < 0) break;
            int s = dA - dB; Poly2 S(dA); S.a.assign(dA + 1, 0); S.a[s] = 1; Q = add2(Q, S);
            Poly2 SB = shl2_nonCirc(B, s); a = add2(a, SB);
        }
        R = a;
    }
    bool invertMod2(const Poly& f, Poly& inv2_out) {
        Poly2 F(G_N); 
        F.a.assign(G_N + 1, 0);
        for (int i = 0; i < G_N; ++i) {
            F.a[i] = (uint8_t)(f[i] & 1);
        }

        Poly2 P(G_N); 
        P.a.assign(G_N + 1, 0); 
        P.a[0] = 1; 
        P.a[G_N] = 1;

        Poly2 a = P, b = F;
        Poly2 ua, va, ub, vb; 
        ua.a = { 1 }; 
        va.a = { 0 }; 
        ub.a = { 0 }; 
        vb.a = { 1 };

        while (true) {
            if (deg2(b) < 0) {
                if (deg2(a) != 0 || a.a[0] != 1) {
                    return false;
                }
                Poly2 Q, R; div2_poly(va, P, Q, R);
                inv2_out.assign(G_N, 0);
                for (int i = 0; i < G_N; ++i) {
                    inv2_out[i] = (i < (int)R.a.size() ? (R.a[i] & 1) : 0);
                }
                return true;
            }
            Poly2 Q, R; 
            div2_poly(a, b, Q, R);

            Poly2 Qu = mul2_nonCirc(Q, ub), Qv = mul2_nonCirc(Q, vb);
            Poly2 n_ub = add2(ua, Qu), n_vb = add2(va, Qv);
            ua = ub; 
            va = vb; 
            ub = n_ub; 
            vb = n_vb; 
            a = b; 
            b = R;
        }
    }
    Poly henselLiftToQ(const Poly& f, const Poly& inv2) {
        Poly inv = inv2; 
        for (int i = 0; i < G_N; ++i) {
            inv[i] &= 1;
        }
        int M = 2;
        while (M < G_Q) {

            Poly t = mulModPow2(inv, f, M);
            int nextM = M << 1; 
            long long mask = (long long)nextM - 1;

            Poly corr(G_N, 0);
            for (int i = 0; i < G_N; ++i) { 
                int ti = t[i] & (M - 1); 
                corr[i] = (2 - ti) & (nextM - 1); 
            }

            inv = mulModPow2(inv, corr, nextM);
            for (int i = 0; i < G_N; ++i) {
                inv[i] = (int)(inv[i] & mask);
            }
            M = nextM;
        }
        Poly res(G_N, 0); 
        for (int i = 0; i < G_N; ++i) {
            res[i] = inv[i] & (G_Q - 1);
        }
        return res;
    }

} // namespace ntru
