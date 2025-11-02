#include "src/model/model.h"

#include <QCryptographicHash>
#include <QFile>
#include <QRandomGenerator>
#include <QRegExp>
#include <QRegularExpression>

#include <algorithm>

/**
 * Load security parameters (N, q, d, ν) from a text file.
 * The file may contain the parameters either space-separated or line-separated,
 * possibly with labels.
 */
bool NtruModel::loadParameters(const QString &paramFile, NtruParams &params) {
    QFile file(paramFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream in(&file);
    params.N = 0;
    params.q = 0;
    params.d = 0;
    params.perturbCount = 0;
    // Read all lines and parse integers
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        // Remove any non-digit (and minus sign) characters (e.g., "N=", "q=" labels)
        static auto qReqExptRemove = QRegularExpression("[^0-9\\- ]");
        line.remove(qReqExptRemove);

        if (line.isEmpty()) continue;

        static auto qReqExptSplit = QRegularExpression("\\s+");
        QStringList tokens = line.split(qReqExptSplit, Qt::SkipEmptyParts);
        for (const QString &tok : tokens) {
            bool ok = false;
          const long long value = tok.toLongLong(&ok);
            if (!ok) continue;
            // Assign values in order if parameters not set yet
            if (params.N == 0) {
                params.N = static_cast<int>(value);
            } else if (params.q == 0) {
                params.q = value;
            } else if (params.d == 0) {
                params.d = static_cast<int>(value);
            } else if (params.perturbCount == 0) {
                params.perturbCount = static_cast<int>(value);
            }
        }
        // If all params filled, break out
        if (params.N != 0 && params.q != 0 && params.d != 0 && params.perturbCount != 0) {
            break;
        }
    }
    file.close();
    // Validate that all required parameters were found
    if (params.N <= 0 || params.q <= 0 || params.d < 0 || params.perturbCount < 0) {
        return false;
    }
    return true;
}

/**
 * Generate a random polynomial of length N with exactly d coefficients of +1 and d coefficients of -1 (ternary polynomial).
 */
static std::vector<Coeff> generateTernaryPoly(int N, int d) {
    std::vector<int> indices(N);
    for (int i = 0; i < N; ++i) indices[i] = i;
    // Shuffle indices to choose random positions
    for (int i = 0; i < 2 * d; ++i) {
        // random index in [i, N-1]
        int j = i + QRandomGenerator::global()->bounded(N - i);
        std::swap(indices[i], indices[j]);
    }
    std::vector<Coeff> poly(N, 0);
    if (2 * d > N) {
        // If 2d > N, capping in case parameters are inconsistent (just fill available slots)
        d = std::min(d, N / 2);
    }
    // First d indices -> +1, next d indices -> -1
    for (int k = 0; k < d; ++k) {
        poly[ indices[k] ] = 1;
    }
    for (int k = 0; k < d; ++k) {
        poly[ indices[d + k] ] = -1;
    }
    return poly;
}

/**
 * Extended Euclidean Algorithm for polynomials: compute gcd(a, b) and coefficients u, v such that u*a + v*b = gcd.
 * (This is a helper for invertPolyMod; it returns gcd and u.)
 */
static std::vector<Coeff> extendedGcdPoly(const std::vector<Coeff> &a, const std::vector<Coeff> &b, Coeff q,
                                         std::vector<Coeff> &u_out) {
    // Represent polynomials as dynamic vectors (without fixed mod X^N yet, for Euclid we treat them in Z_q[X] context)
    std::vector<Coeff> A = a;
    std::vector<Coeff> B = b;
    // Remove leading zeros if any (polynomials normalized)
    auto trim = [&](std::vector<Coeff>& poly) {
        while (!poly.empty() && poly.back() % q == 0) { // mod q: treat 0
            poly.pop_back();
        }
        if (poly.empty()) poly.push_back(0);
    };
    trim(A);
    trim(B);
    // Initialize u0=1, v0=0 for A; u1=0, v1=1 for B
    std::vector<Coeff> u0 = {1}, v0 = {0};
    std::vector<Coeff> u1 = {0}, v1 = {1};

    // Helper to perform polynomial division (dividing polyX by polyY, returning quotient and remainder).
    auto polyDivMod = [&](const std::vector<Coeff>& X, const std::vector<Coeff>& Y) {
        std::vector<Coeff> QQ;  // quotient
        std::vector<Coeff> RR = X;  // remainder initially X
        // Ensure Y is trimmed (leading coeff non-zero)
        std::vector<Coeff> D = Y;
        trim(const_cast<std::vector<Coeff>&>(D));
        int degY = D.size() - 1;
        // Precompute inverse of leading coefficient of D mod q (assuming q prime or coprime scenario)
        Coeff leadInv = 0;
        Coeff lead = D.back() % q;
        if (lead < 0) lead += q;
        // Find modular inverse of lead in Z_q if possible
        // Simple method: brute for small q or extended Euclid on integers
        if (lead != 0) {
            // extended Euclid on ints for lead and q (since q might not be prime, but if gcd(lead,q)=1)
            // We'll attempt only if invertible
            Coeff t0 = 0, t1 = 1;
            Coeff r0 = q, r1 = lead;
            while (r1 != 0) {
                Coeff div = r0 / r1;
                Coeff r2 = r0 - div * r1;
                r0 = r1;
                r1 = r2;
                Coeff t2 = t0 - div * t1;
                t0 = t1;
                t1 = t2;
            }
            if (r0 == 1 || r0 == -1) {
                // t0 is inverse of lead mod q (reduce into 0..q-1)
                leadInv = t0 % q;
                if (leadInv < 0) leadInv += q;
            } else {
                leadInv = 0; // not invertible
            }
        }
        int degX = RR.size() - 1;
        // If division degree feasible
        while (leadInv != 0 && !RR.empty() && (RR.size() - 1) >= degY && RR.back() != 0) {
            // Determine factor to cancel highest term of RR
            int degR = RR.size() - 1;
            Coeff coeffR = RR.back() % q;
            if (coeffR < 0) coeffR += q;
            // scale = coeffR / lead (mod q)
            Coeff scale = (coeffR * leadInv) % q;
            // Degree difference
            int shift = degR - degY;
            // Ensure quotient has size shift+1
            if (QQ.size() < shift + 1) {
                QQ.resize(shift + 1);
            }
            QQ[shift] = (QQ[shift] + scale) % q;
            // Subtract scale * D * x^shift from RR
            // Expand RR length if needed for subtraction
            if (RR.size() < D.size() + shift) {
                RR.resize(D.size() + shift);
            }
            // Subtract each term
            for (int j = 0; j < D.size(); ++j) {
                Coeff sub = (scale * D[j]) % q;
                // Align at position j + shift
                int idx = j + shift;
                RR[idx] = (RR[idx] - sub) % q;
                if (RR[idx] < 0) RR[idx] += q;
            }
            trim(RR);
        }
        if (RR.size() == 0) {
            RR.push_back(0);
        }
        trim(RR);
        // Ensure quotient trimmed
        trim(QQ);
        return std::pair<std::vector<Coeff>, std::vector<Coeff>>(QQ, RR);
    };

    // Extended Euclidean loop
    while (true) {
        // If A is 0, gcd is B
        trim(A);
        trim(B);
        if (A.size() == 1 && (A[0] % q + q) % q == 0) {
            // gcd is B
            u_out = u1;  // corresponding u for B
            return B;
        }
        // If B is 0, gcd is A
        if (B.size() == 1 && (B[0] % q + q) % q == 0) {
            // gcd is A
            u_out = u0;
            return A;
        }
        // Compute quotient and remainder of B / A
        auto divRes = polyDivMod(B, A);
        std::vector<Coeff> Q = divRes.first;
        std::vector<Coeff> R = divRes.second;
        // Compute new combination coefficients for remainder:
        // r = B - Q * A, so
        // u_r = u1 - Q * u0, v_r = v1 - Q * v0.
        // Perform polynomial multiplication Q * u0 and Q * v0
        // We need to compute Q * u0 and Q * v0 mod q (no X^N-1 reduction here).
        auto polyMultiply = [&](const std::vector<Coeff>& P, const std::vector<Coeff>& QP) {
            std::vector<Coeff> product(P.size() + QP.size() - 1);
            for (int i = 0; i < P.size(); ++i) {
                Coeff cPi = P[i] % q;
                if (cPi < 0) cPi += q;
                for (int j = 0; j < QP.size(); ++j) {
                    Coeff cQj = QP[j] % q;
                    if (cQj < 0) cQj += q;
                    __int128 mul = (__int128)cPi * cQj;
                    Coeff c = (Coeff)(mul % q);
                    product[i + j] += c;
                    product[i + j] %= q;
                    if (product[i + j] < 0) product[i + j] += q;
                }
            }
            // Trim any leading zeros
            while (product.size() > 1 && product.back() % q == 0) {
                product.pop_back();
            }
            if (product.empty()) product.push_back(0);
            return product;
        };
        std::vector<Coeff> Qu0 = polyMultiply(Q, u0);
        std::vector<Coeff> Qv0 = polyMultiply(Q, v0);
        // u_r = u1 - Qu0, v_r = v1 - Qv0
        // Ensure same length for subtraction
        size_t len = std::max(u1.size(), Qu0.size());
        std::vector<Coeff> u_r(len);
        for (size_t i = 0; i < len; ++i) {
            Coeff c1 = 0, c2 = 0;
            if (i < u1.size()) {
                c1 = u1[i] % q;
            }
            if (i < Qu0.size()) {
                c2 = Qu0[i] % q;
            }
            Coeff diff = c1 - c2;
            diff %= q;
            if (diff < 0) diff += q;
            u_r[i] = diff;
        }
        while (u_r.size() > 1 && u_r.back() == 0) {
            u_r.pop_back();
        }
        len = std::max(v1.size(), Qv0.size());
        std::vector<Coeff> v_r(len);
        for (size_t i = 0; i < len; ++i) {
            Coeff c1 = 0, c2 = 0;
            if (i < v1.size()) c1 = v1[i] % q;
            if (i < Qv0.size()) c2 = Qv0[i] % q;
            Coeff diff = c1 - c2;
            diff %= q;
            if (diff < 0) diff += q;
            v_r[i] = diff;
        }
        while (v_r.size() > 1 && v_r.back() == 0) {
            v_r.pop_back();
        }
        // Update: B = A, A = R; accordingly update (u1,v1) = (u0,v0), (u0,v0) = (u_r,v_r)
        B = A;
        A = R;
        v1 = v0;
        u1 = u0;
        v0 = v_r;
        u0 = u_r;
    }
}

/**
 * Compute the multiplicative inverse of polynomial a modulo (X^N - 1, q), i.e., find a_inv such that a * a_inv ≡ 1 (mod q, X^N-1).
 * Returns true on success (a_inv is filled), or false if no inverse exists.
 */
bool NtruModel::invertPolyMod(const std::vector<Coeff> &a, const NtruParams &params, std::vector<Coeff> &a_inv) {
    // We need to find polynomial u such that u * a + v * (x^N - 1) = d, where d (the gcd) should be 1 for invertibility:contentReference[oaicite:3]{index=3}.
    // Represent polynomial (x^N - 1) as B = x^N - 1 (coeffs: 1 at degree N, -1 at degree 0).
    std::vector<Coeff> polyB(params.N + 1);
    polyB[params.N] = 1;
    polyB[0] = -1;
    std::vector<Coeff> u;
    std::vector<Coeff> gcd = extendedGcdPoly(a, polyB, params.q, u);
    // gcd now holds gcd(a, x^N-1). For invertibility in the ring, gcd must be a nonzero constant (e.g., 1) that is invertible mod q.
    // Check if gcd is a constant polynomial
    if (gcd.size() == 0) {
        return false;
    }
    // Reduce gcd constant mod q
    Coeff gconst = gcd[0] % params.q;
    if (gconst < 0) gconst += params.q;
    // If gcd has degree > 0 or constant is 0, no inverse.
    if (gcd.size() > 1 || gconst == 0) {
        return false;
    }
    // If constant gcd not 1, multiply u by inverse of gcd constant.
    if (gconst != 1) {
        // Compute multiplicative inverse of gconst mod q (since q might not be prime, this exists only if gconst is coprime with q)
        // We can use Euclidean algorithm on integers to find inverse of gconst mod q.
        Coeff t0 = 0, t1 = 1;
        Coeff r0 = params.q, r1 = gconst;
        while (r1 != 0) {
            Coeff div = r0 / r1;
            Coeff newr = r0 - div * r1;
            r0 = r1;
            r1 = newr;
            Coeff newt = t0 - div * t1;
            t0 = t1;
            t1 = newt;
        }
        if (r0 != 1 && r0 != -1) {
            return false; // no inverse for gconst mod q
        }
        Coeff invConst = t0 % params.q;
        if (invConst < 0) invConst += params.q;
        // Multiply polynomial u by invConst mod q
        for (Coeff &coeff : u) {
            coeff = (coeff * invConst) % params.q;
            if (coeff < 0) coeff += params.q;
        }
    }
    // At this point, u * a ≡ 1 (mod x^N-1, q). Now reduce u mod (x^N-1) to get inverse in standard form.
    // We need to produce a polynomial of degree < N that is congruent to u modulo (x^N - 1).
    a_inv.assign(params.N, 0);
    // For each term in u, "wrap" terms with degree >= N back into [0, N-1] range using x^N ≡ 1.
    for (int i = 0; i < u.size(); ++i) {
        Coeff coeff = u[i] % params.q;
        if (coeff == 0) continue;
        // Compute index mod N
        int idx = i % params.N;
        // If i >= N, using x^N = 1, x^i = x^(idx) * (x^N)^(i/N) = x^idx * 1^(i/N) = x^idx.
        // So just accumulate at index idx.
        a_inv[idx] = (a_inv[idx] + coeff) % params.q;
        if (a_inv[idx] < 0) a_inv[idx] += params.q;
    }
    return true;
}

/**
 * Polynomial addition res = a + b (mod q). Assumes a and b are length N.
 */
void NtruModel::polyAdd(std::vector<Coeff> &res, const std::vector<Coeff> &a, const std::vector<Coeff> &b, Coeff q) {
    int N = a.size();
    res.resize(N);
    for (int i = 0; i < N; ++i) {
        Coeff c = a[i] + b[i];
        c %= q;
        if (c < 0) c += q;
        res[i] = c;
    }
}

/**
 * Polynomial subtraction res = a - b (mod q). Assumes a and b are length N.
 */
void NtruModel::polySub(std::vector<Coeff> &res, const std::vector<Coeff> &a, const std::vector<Coeff> &b, Coeff q) {
    int N = a.size();
    res.resize(N);
    for (int i = 0; i < N; ++i) {
        Coeff c = a[i] - b[i];
        c %= q;
        if (c < 0) c += q;
        res[i] = c;
    }
}

/**
 * Polynomial multiplication res = a * b (mod X^N - 1, mod q).
 * Uses convolution with wrap-around for mod (X^N-1).
 */
void NtruModel::polyMulMod(std::vector<Coeff> &res, const std::vector<Coeff> &a, const std::vector<Coeff> &b, const NtruParams &params) {
    int N = params.N;
    res.assign(N, 0);
    // Convolution: (a * b)[k] = sum_{i+j ≡ k (mod N)} a[i]*b[j].
    for (int i = 0; i < N; ++i) {
        Coeff ai = a[i] % params.q;
        if (ai < 0) ai += params.q;
        if (ai == 0) continue;
        for (int j = 0; j < N; ++j) {
            Coeff bj = b[j] % params.q;
            if (bj < 0) bj += params.q;
            if (bj == 0) continue;
            // (i + j) mod N is the resulting index
            int k = i + j;
            if (k >= N) {
                k -= N;
            }
            __int128 prod = (__int128)ai * bj;
            Coeff c = (Coeff)(prod % params.q);
            res[k] += c;
            res[k] %= params.q;
            if (res[k] < 0) res[k] += params.q;
        }
    }
}

/**
 * Center the polynomial coefficients to the range [ -floor(q/2), floor(q/2) ].
 */
void NtruModel::centerPoly(std::vector<Coeff> &poly, Coeff q) {
    for (Coeff &c : poly) {
        // Bring into range 0..q-1 first
        c %= q;
        if (c < 0) c += q;
        // Adjust to [-q/2, q/2]
        if (c > q/2) {
            c -= q;
        }
    }
}

/**
 * Generate a key pair (private and public keys) given NTRU parameters.
 */
bool NtruModel::generateKeyPair(const NtruParams &params, NtruPrivKey &privKey, NtruPubKey &pubKey) {
    int N = params.N;
    // Generate random f and g until f is invertible mod (X^N-1, q)
    std::vector<Coeff> f, g;
    std::vector<Coeff> f_inv;
    const int MAX_TRIES = 10;
    bool invertible = false;
    for (int attempt = 0; attempt < MAX_TRIES; ++attempt) {
        f = generateTernaryPoly(N, params.d);
        // Ensure f is invertible mod q
        if (!invertPolyMod(f, params, f_inv)) {
            continue;
        }
        // Also ensure f is invertible mod q (the above ensures mod X^N-1 as well)
        invertible = true;
        break;
    }
    if (!invertible) {
        return false; // failed to find invertible f
    }
    // Generate g as another random small polynomial (ternary with same weight d)
    g = generateTernaryPoly(N, params.d);
    // Compute public key h = f_inv * g (mod X^N-1, q):contentReference[oaicite:4]{index=4}
    std::vector<Coeff> h;
    polyMulMod(h, f_inv, g, params);
    // Generate perturbation vectors e_i
    std::vector< std::vector<Coeff> > eList;
    for (int i = 0; i < params.perturbCount; ++i) {
        std::vector<Coeff> e = generateTernaryPoly(N, params.d);
        eList.push_back(e);
    }
    // Assign keys
    privKey.f = f;
    privKey.g = g;
    privKey.eList = eList;
    pubKey.h = h;
    return true;
}

/**
 * Save private key to a text file. Format:
 * f (N coefficients on one line),
 * g (N coefficients on next line),
 * followed by ν lines for each perturbation polynomial (N coefficients each).
 */
bool NtruModel::savePrivateKey(const QString &privKeyFile, const NtruParams &params, const NtruPrivKey &privKey) {
    QFile file(privKeyFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&file);
    // Save f
    for (int j = 0; j < params.N; ++j) {
        // Store coefficient in [0, q-1] for consistency
        Coeff c = privKey.f[j] % params.q;
        if (c < 0) c += params.q;
        out << c;
        if (j < params.N - 1) out << " ";
    }
    out << "\n";
    // Save g
    for (int j = 0; j < params.N; ++j) {
        Coeff c = privKey.g[j] % params.q;
        if (c < 0) c += params.q;
        out << c;
        if (j < params.N - 1) out << " ";
    }
    out << "\n";
    // Save each perturbation polynomial
    for (int i = 0; i < privKey.eList.size(); ++i) {
        const std::vector<Coeff> &e = privKey.eList[i];
        for (int j = 0; j < params.N; ++j) {
            Coeff c = e[j] % params.q;
            if (c < 0) c += params.q;
            out << c;
            if (j < params.N - 1) out << " ";
        }
        out << "\n";
    }
    file.close();
    return true;
}

/**
 * Save public key to a text file. Format: one line with N coefficients of h (space-separated).
 */
bool NtruModel::savePublicKey(const QString &pubKeyFile, const NtruParams &params, const NtruPubKey &pubKey) {
    QFile file(pubKeyFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&file);
    for (int j = 0; j < params.N; ++j) {
        Coeff c = pubKey.h[j] % params.q;
        if (c < 0) c += params.q;
        out << c;
        if (j < params.N - 1) out << " ";
    }
    out << "\n";
    file.close();
    return true;
}

/**
 * Load private key from file (given known parameters N and ν). Expects the same format as saved by savePrivateKey.
 */
bool NtruModel::loadPrivateKey(const QString &privKeyFile, const NtruParams &params, NtruPrivKey &privKey) {
    static auto qReqExptSplit = QRegularExpression("\\s+");

    QFile file(privKeyFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream in(&file);
    QString line;
    // Read f
    if (in.readLineInto(&line) == false) {
        return false;
    }

    QStringList coeffs = line.split(qReqExptSplit, Qt::SkipEmptyParts);
    if (coeffs.size() < params.N) {
        return false;
    }
    privKey.f.resize(params.N);
    for (int j = 0; j < params.N; ++j) {
        bool ok;
        long long val = coeffs[j].toLongLong(&ok);
        if (!ok) {
            return false;
        }
        // We can store possibly in symmetric representation for internal use
        Coeff c = val % params.q;
        if (c > params.q/2) c -= params.q;
        privKey.f[j] = c;
    }
    // Read g
    if (in.readLineInto(&line) == false) {
        return false;
    }
    coeffs = line.split(qReqExptSplit, Qt::SkipEmptyParts);
    if (coeffs.size() < params.N) {
        return false;
    }
    privKey.g.resize(params.N);
    for (int j = 0; j < params.N; ++j) {
        bool ok;
        long long val = coeffs[j].toLongLong(&ok);
        if (!ok) {
            return false;
        }
        Coeff c = val % params.q;
        if (c > params.q/2) c -= params.q;
        privKey.g[j] = c;
    }
    // Read perturbation polynomials
    privKey.eList.clear();
    for (int i = 0; i < params.perturbCount; ++i) {
        if (in.readLineInto(&line) == false) {
            return false;
        }
        coeffs = line.split(qReqExptSplit, Qt::SkipEmptyParts);
        if (coeffs.size() < params.N) {
            return false;
        }
        std::vector<Coeff> e(params.N);
        for (int j = 0; j < params.N; ++j) {
            bool ok;
            long long val = coeffs[j].toLongLong(&ok);
            if (!ok) {
                return false;
            }
            Coeff c = val % params.q;
            if (c > params.q/2) c -= params.q;
            e[j] = c;
        }
        privKey.eList.push_back(e);
    }
    file.close();
    return true;
}

/**
 * Load public key from file. Expects N space-separated coefficients.
 */
bool NtruModel::loadPublicKey(const QString &pubKeyFile, const NtruParams &params, NtruPubKey &pubKey) {
    static auto qReqExptSplit = QRegularExpression("\\s+");

    QFile file(pubKeyFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream in(&file);
    QString line;
    if (in.readLineInto(&line) == false) {
        return false;
    }
    QStringList coeffs = line.split(qReqExptSplit, Qt::SkipEmptyParts);
    if (coeffs.size() < params.N) {
        return false;
    }
    pubKey.h.resize(params.N);
    for (int j = 0; j < params.N; ++j) {
        bool ok;
        long long val = coeffs[j].toLongLong(&ok);
        if (!ok) {
            return false;
        }
        Coeff c = val % params.q;
        if (c > params.q/2) c -= params.q;
        pubKey.h[j] = c;
    }
    file.close();
    return true;
}

/**
 * Sign the message file using the provided private key and parameters. Saves the signature (polynomial) to signatureFile.
 */
bool NtruModel::signMessage(const NtruParams &params, const NtruPrivKey &privKey,
                            const QString &messageFile, const QString &signatureFile) {
    // Read entire message file
    QFile file(messageFile);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    QByteArray messageData = file.readAll();
    file.close();
    // Hash the message using SHA-256
    QByteArray hash = QCryptographicHash::hash(messageData, QCryptographicHash::Sha256);
    // Map hash to a polynomial of length N:
    std::vector<Coeff> mPoly(params.N);
    const int hashLen = hash.size();  // 32 bytes for SHA-256
    for (int i = 0; i < params.N; ++i) {
        // Use hash bytes cyclically as coefficients (mod q)
        unsigned char byteVal = hash.at(i % hashLen);
        Coeff c = static_cast<Coeff>(byteVal);
        c %= params.q;
        // For variety, center it in -128..127 if q > 256 (for smaller q, this still yields 0..q-1 range)
        if (c > params.q/2) c -= params.q;
        mPoly[i] = c;
    }
    // Compute f_inv if not stored (we can derive from f and h: h = f^(-1)*g mod q, but direct f_inv needed)
    std::vector<Coeff> f_inv;
    if (!invertPolyMod(privKey.f, params, f_inv)) {
        return false; // should not happen if keys are valid
    }
    // Choose random combination of perturbation vectors: e_sum = sum_{i=1..ν} a_i * e_i, where a_i ∈ {-1,0,1}.
    std::vector<Coeff> e_sum(params.N, 0);
    if (params.perturbCount > 0) {
        // Randomly pick coefficients -1, 0, or 1 for each perturbation vector
        for (int i = 0; i < params.perturbCount; ++i) {
            int randChoice = QRandomGenerator::global()->bounded(3) - 1; // yields -1, 0, or 1
            if (randChoice == 0) continue;
            // e_sum += randChoice * eList[i]
            for (int j = 0; j < params.N; ++j) {
                Coeff add = randChoice * privKey.eList[i][j];
                e_sum[j] += add;
            }
        }
        // Reduce e_sum coefficients mod q to safe range
        for (int j = 0; j < params.N; ++j) {
            e_sum[j] %= params.q;
            if (e_sum[j] < 0) e_sum[j] += params.q;
            // Center it around 0
            if (e_sum[j] > params.q/2) e_sum[j] -= params.q;
        }
    }
    // Compute v_sum = e_sum * h (mod X^N-1, q)
    std::vector<Coeff> v_sum;
    polyMulMod(v_sum, e_sum, privKey.g, params);  // Actually e_sum * h mod q = e_sum * (f_inv*g) = (e_sum*f_inv)*g, but easier to use relation f*h = g.
    // Note: We can use relation f * h ≡ g to get v_sum = e_sum * h mod q =? (e_sum * g / f) mod, but direct multiply e_sum by g mod q might not equal e_sum*h since h = f_inv*g mod q.
    // Actually better: h = f_inv * g, so e_sum * h = e_sum * f_inv * g. But computing e_sum * f_inv mod might be easier:
    // We'll instead do: polyMulMod(temp, e_sum, f_inv, params); then polyMulMod(v_sum, temp, privKey.g, params). This yields e_sum * f_inv * g = e_sum * h mod q.
    std::vector<Coeff> temp;
    polyMulMod(temp, e_sum, f_inv, params);
    polyMulMod(v_sum, temp, privKey.g, params);
    // Compute target = mPoly + v_sum (mod q)
    std::vector<Coeff> target(params.N);
    polyAdd(target, mPoly, v_sum, params.q);
    // Compute u_base = f_inv * target (mod X^N-1, q)
    std::vector<Coeff> u_base;
    polyMulMod(u_base, f_inv, target, params);
    // Now our lattice solution: u_total = u_base - e_sum (the first component of the lattice vector):contentReference[oaicite:5]{index=5}.
    std::vector<Coeff> u_total(params.N);
    polySub(u_total, u_base, e_sum, params.q);
    // Center coefficients of u_total to get a short representative
    centerPoly(u_total, params.q);
    // (Optional) Also center v_total if needed (not output, but can be computed for completeness):
    // Actually v_total = u_total * h mod q which should equal mPoly mod q. We can ignore computing v_total explicitly.
    // Save signature polynomial u_total to file
    QFile sigFile(signatureFile);
    if (!sigFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&sigFile);
    for (int j = 0; j < params.N; ++j) {
        // We save the signature coefficients in centered form
        Coeff c = u_total[j];
        // Ensure a specific representative within [-(q/2), q/2]
        if (c < 0) {
            // For negative, convert to equivalent positive mod q for storage (to avoid '-' in text if not desired, but it's fine to keep negatives too)
            // We'll store as integer possibly negative, it's okay since we'll parse as long long.
        }
        out << c;
        if (j < params.N - 1) out << " ";
    }
    out << "\n";
    sigFile.close();
    return true;
}

/**
 * Verify the signature of a message using the public key. Returns true if signature is valid.
 */
bool NtruModel::verifyMessage(const NtruParams &params, const NtruPubKey &pubKey,
                              const QString &messageFile, const QString &signatureFile) {
    static auto qReqExptSplit = QRegularExpression("\\s+");

    // Read message and compute hash polynomial as in signMessage
    QFile file(messageFile);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    QByteArray messageData = file.readAll();
    file.close();
    QByteArray hash = QCryptographicHash::hash(messageData, QCryptographicHash::Sha256);
    std::vector<Coeff> mPoly(params.N);
    const int hashLen = hash.size();
    for (int i = 0; i < params.N; ++i) {
        unsigned char byteVal = hash.at(i % hashLen);
        Coeff c = static_cast<Coeff>(byteVal);
        c %= params.q;
        if (c > params.q/2) c -= params.q;
        mPoly[i] = c;
    }
    // Load signature polynomial from file
    QFile sigFile(signatureFile);
    if (!sigFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream in(&sigFile);
    QString line;
    if (!in.readLineInto(&line)) {
        return false;
    }
    sigFile.close();
    QStringList coeffs = line.split(qReqExptSplit, Qt::SkipEmptyParts);
    if (coeffs.size() < params.N) {
        return false;
    }
    std::vector<Coeff> sigPoly(params.N);
    for (int j = 0; j < params.N; ++j) {
        bool ok;
        long long val = coeffs[j].toLongLong(&ok);
        if (!ok) {
            return false;
        }
        // Bring into [0,q-1] for multiplication
        Coeff c = val % params.q;
        if (c < 0) c += params.q;
        sigPoly[j] = c;
    }
    // Compute v' = sigPoly * h (mod X^N-1, q)
    std::vector<Coeff> v_check;
    polyMulMod(v_check, sigPoly, pubKey.h, params);
    // Now check if v_check ≡ mPoly (mod q). They are both length N polynomials.
    bool valid = true;
    for (int j = 0; j < params.N; ++j) {
        // Compare in mod q sense
        Coeff a = mPoly[j] % params.q;
        Coeff b = v_check[j] % params.q;
        if (a < 0) a += params.q;
        if (b < 0) b += params.q;
        // We consider them equal mod q if difference mod q is 0
        if ((a - b) % params.q != 0) {
            valid = false;
            break;
        }
    }
    return valid;
}
