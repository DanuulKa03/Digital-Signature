#include "src/model/model.h"
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QCryptographicHash>
#include <QtCore/QDataStream>
#include <QtCore/QVector>
#include <QtCore/QByteArray>
#include <QtCore/QIODevice>
#include <cmath>

// ===== ctor =====
NtruModel::NtruModel() = default;

// ===== утилиты модульной арифметики =====
int NtruModel::modQ(long long x, int q) {
    long long t = x % q;
    if (t < 0) t += q;
    return static_cast<int>(t);
}
int NtruModel::center(int a, int q) {
    int v = a % q; if (v < 0) v += q;
    if (v > q / 2) v -= q;
    return v;
}
NtruModel::Poly NtruModel::zeroP() const { return Poly(G_N, 0); }

NtruModel::Poly NtruModel::subMod(const Poly& A, const Poly& B) const {
    Poly R(G_N, 0);
    for (int i = 0; i < G_N; ++i) R[i] = modQ(static_cast<long long>(A[i]) - B[i], G_Q);
    return R;
}
NtruModel::Poly NtruModel::mulModQ(const Poly& A, const Poly& B) const {
    QVector<long long> acc(G_N, 0);
    for (int i = 0; i < G_N; ++i) if (A[i])
            for (int j = 0; j < G_N; ++j) if (B[j]) {
                    int k = i + j; if (k >= G_N) k -= G_N;
                    acc[k] += static_cast<long long>(A[i]) * B[j];
                }
    Poly R(G_N, 0);
    for (int i = 0; i < G_N; ++i) R[i] = modQ(acc[i], G_Q);
    return R;
}
NtruModel::Poly NtruModel::mulModPow2(const Poly& A, const Poly& B, int M) const {
    long long mask = static_cast<long long>(M) - 1;
    QVector<long long> acc(G_N, 0);
    for (int i = 0; i < G_N; ++i) if ((A[i] & mask) != 0)
            for (int j = 0; j < G_N; ++j) if ((B[j] & mask) != 0) {
                    int k = i + j; if (k >= G_N) k -= G_N;
                    acc[k] += static_cast<long long>(A[i]) * B[j];
                }
    Poly R(G_N, 0);
    for (int i = 0; i < G_N; ++i) R[i] = static_cast<int>(acc[i] & mask);
    return R;
}

// ===== GF(2) + Хензель =====
int NtruModel::deg2(const Poly2& p) {
    for (int i = p.a.size() - 1; i >= 0; --i) if (p.a[i]) return i;
    return -1;
}
NtruModel::Poly2 NtruModel::trim2(const Poly2& p) {
    int d = deg2(p);
    Poly2 r; r.a = QVector<unsigned char>(qMax(d, 0) + 1, 0);
    for (int i = 0; i <= d; ++i) r.a[i] = p.a[i];
    return r;
}
NtruModel::Poly2 NtruModel::add2(const Poly2& A, const Poly2& B) {
    int m = qMax(A.a.size(), B.a.size());
    Poly2 R; R.a = QVector<unsigned char>(m, 0);
    for (int i = 0; i < m; ++i) {
        unsigned char va = (i < A.a.size() ? A.a[i] : 0);
        unsigned char vb = (i < B.a.size() ? B.a[i] : 0);
        R.a[i] = static_cast<unsigned char>(va ^ vb);
    }
    return trim2(R);
}
NtruModel::Poly2 NtruModel::shl2_nonCirc(const Poly2& A, int k) {
    if (k <= 0) return A;
    Poly2 R; R.a = QVector<unsigned char>(A.a.size() + k, 0);
    for (int i = 0; i < A.a.size(); ++i) if (A.a[i]) R.a[i + k] ^= 1;
    return R;
}
NtruModel::Poly2 NtruModel::mul2_nonCirc(const Poly2& A, const Poly2& B) {
    Poly2 R; R.a = QVector<unsigned char>(A.a.size() + B.a.size() + 1, 0);
    for (int i = 0; i < A.a.size(); ++i) if (A.a[i])
            for (int j = 0; j < B.a.size(); ++j) if (B.a[j]) {
                    int idx = i + j;
                    if (idx >= R.a.size()) R.a.resize(idx + 1, 0);
                    R.a[idx] ^= 1;
                }
    return trim2(R);
}
void NtruModel::div2_poly(const Poly2& A, const Poly2& B, Poly2& Q, Poly2& R) {
    Poly2 a = A; Q.a = QVector<unsigned char>(qMax(1, A.a.size()), 0);
    int dB = deg2(B); if (dB < 0) { R = a; return; }
    while (true) {
        int dA = deg2(a); if (dA < dB || dA < 0) break;
        int s = dA - dB;
        Poly2 S; S.a = QVector<unsigned char>(dA + 1, 0); S.a[s] = 1;
        Q = add2(Q, S);
        Poly2 SB = shl2_nonCirc(B, s);
        a = add2(a, SB);
    }
    R = a;
}
bool NtruModel::invertMod2(const Poly& f, Poly& inv2_out) const {
    Poly2 F; F.a = QVector<unsigned char>(G_N + 1, 0);
    for (int i = 0; i < G_N; ++i) F.a[i] = static_cast<unsigned char>(f[i] & 1);
    Poly2 P; P.a = QVector<unsigned char>(G_N + 1, 0); P.a[0] = 1; P.a[G_N] = 1;

    Poly2 a = P, b = F, ua, va, ub, vb;
    ua.a = { 1 }; va.a = { 0 }; ub.a = { 0 }; vb.a = { 1 };

    while (true) {
        if (deg2(b) < 0) {
            if (deg2(a) != 0 || a.a[0] != 1) return false;
            Poly2 Q, R; div2_poly(va, P, Q, R);
            inv2_out = Poly(G_N, 0);
            for (int i = 0; i < G_N; ++i) inv2_out[i] = (i < R.a.size() ? (R.a[i] & 1) : 0);
            return true;
        }
        Poly2 Q, R; div2_poly(a, b, Q, R);
        Poly2 Qu = mul2_nonCirc(Q, ub), Qv = mul2_nonCirc(Q, vb);
        Poly2 n_ub = add2(ua, Qu), n_vb = add2(va, Qv);
        ua = ub; va = vb; ub = n_ub; vb = n_vb; a = b; b = R;
    }
}
NtruModel::Poly NtruModel::henselLiftToQ(const Poly& f, const Poly& inv2) const {
    Poly inv = inv2;
    for (int i = 0; i < G_N; ++i) inv[i] &= 1;
    int M = 2;
    while (M < G_Q) {
        Poly t = mulModPow2(inv, f, M);
        int nextM = M << 1;
        long long mask = static_cast<long long>(nextM) - 1;
        Poly corr(G_N, 0);
        for (int i = 0; i < G_N; ++i) {
            int ti = t[i] & (M - 1);
            corr[i] = (2 - ti) & (nextM - 1);
        }
        inv = mulModPow2(inv, corr, nextM);
        for (int i = 0; i < G_N; ++i) inv[i] = static_cast<int>(inv[i] & mask);
        M = nextM;
    }
    Poly res(G_N, 0);
    for (int i = 0; i < G_N; ++i) res[i] = inv[i] & (G_Q - 1);
    return res;
}

// ===== RNG (ChaCha20) =====
void NtruModel::ChaCha20::set(const uchar key[32], const uchar nonce[12], quint32 counter) {
    st[0]=0x61707865u; st[1]=0x3320646eu; st[2]=0x79622d32u; st[3]=0x6b206574u;
    for (int i=0;i<8;++i) {
        st[4+i] = static_cast<quint32>(key[4*i])
                  | (static_cast<quint32>(key[4*i+1])<<8)
                  | (static_cast<quint32>(key[4*i+2])<<16)
                  | (static_cast<quint32>(key[4*i+3])<<24);
    }
    st[12]=counter;
    st[13]= static_cast<quint32>(nonce[0]) | (static_cast<quint32>(nonce[1])<<8) |
            (static_cast<quint32>(nonce[2])<<16) | (static_cast<quint32>(nonce[3])<<24);
    st[14]= static_cast<quint32>(nonce[4]) | (static_cast<quint32>(nonce[5])<<8) |
            (static_cast<quint32>(nonce[6])<<16) | (static_cast<quint32>(nonce[7])<<24);
    st[15]= static_cast<quint32>(nonce[8]) | (static_cast<quint32>(nonce[9])<<8) |
            (static_cast<quint32>(nonce[10])<<16)| (static_cast<quint32>(nonce[11])<<24);
}
void NtruModel::ChaCha20::block(uchar out[64]) {
    quint32 x[16]; for (int i=0;i<16;++i) x[i]=st[i];
    for (int i=0;i<10;++i) {
        qr(x,0,4,8,12); qr(x,1,5,9,13); qr(x,2,6,10,14); qr(x,3,7,11,15);
        qr(x,0,5,10,15); qr(x,1,6,11,12); qr(x,2,7,8,13); qr(x,3,4,9,14);
    }
    for (int i=0;i<16;++i) x[i]+=st[i];
    for (int i=0;i<16;++i) {
        out[4*i+0]= static_cast<uchar>(x[i]);
        out[4*i+1]= static_cast<uchar>(x[i]>>8);
        out[4*i+2]= static_cast<uchar>(x[i]>>16);
        out[4*i+3]= static_cast<uchar>(x[i]>>24);
    }
    st[12]+=1;
}

NtruModel::SecureChaChaRng::SecureChaChaRng() { reseed(nullptr, 0); }

void NtruModel::SecureChaChaRng::reseed(const void* extra, size_t elen) {
    // простая энтропия: время и адреса (для демонстрационной цели)
    QByteArray seed;
    seed.append(reinterpret_cast<const char*>(&extra), sizeof(extra));
    quint64 t1 = static_cast<quint64>(QDateTime::currentMSecsSinceEpoch());
    seed.append(reinterpret_cast<const char*>(&t1), sizeof(t1));
    if (extra && elen) seed.append(reinterpret_cast<const char*>(extra), static_cast<int>(elen));

    QByteArray dig1 = NtruModel::sha256(seed);
    QByteArray mat  = dig1 + QByteArray("nonce-01", 8);
    QByteArray dig2 = NtruModel::sha256(mat);

    uchar key[32];  std::memcpy(key,  dig1.constData(), 32);
    uchar nonce[12];std::memcpy(nonce,dig2.constData(), 12);
    ch.set(key, nonce, 1);
    idx = 64;
}

quint32 NtruModel::SecureChaChaRng::operator()() {
    if (idx >= 64) { uchar block[64]; ch.block(block); std::memcpy(buf, block, 64); idx = 0; }
    quint32 v = static_cast<quint32>(buf[idx]) |
                (static_cast<quint32>(buf[idx+1])<<8) |
                (static_cast<quint32>(buf[idx+2])<<16)|
                (static_cast<quint32>(buf[idx+3])<<24);
    idx += 4;
    return v;
}

// ===== QCryptographicHash helpers =====
QByteArray NtruModel::sha256(const QByteArray& data) {
    return QCryptographicHash::hash(data, QCryptographicHash::Sha256);
}

void NtruModel::xofFill(const QByteArray& seed, uchar* out, size_t len) {
    QByteArray ctr(4, '\0');
    size_t off = 0;
    while (off < len) {
        QByteArray input = seed + ctr;
        QByteArray d = sha256(input);
        const int take = static_cast<int>(qMin(len - off, static_cast<size_t>(d.size())));
        std::memcpy(out + off, d.constData(), take);
        off += take;

        // ++ctr (big-endian)
        for (int i = 3; i >= 0; --i) {
            unsigned char c = static_cast<unsigned char>(ctr[i]);
            c += 1;
            ctr[i] = static_cast<char>(c);
            if (c != 0) break;
        }
    }
}

// ===== keygen helpers =====
void NtruModel::genTernary(Poly& a, const std::function<quint32()>& rng) {
    a = Poly(G_N, 0);
    QVector<int> idx(G_N);
    for (int i=0;i<G_N;++i) idx[i]=i;
    for (int i=G_N-1;i>0;--i) {
        quint32 r = rng() % static_cast<quint32>(i+1);
        qSwap(idx[i], idx[r]);
    }
    int plus = G_D/2;
    int minus = G_D - plus;
    if (plus == minus) { plus--; minus++; }
    for (int i=0;i<plus;++i)  a[idx[i]] = 1;
    for (int i=plus;i<plus+minus;++i) a[idx[i]] = G_Q - 1;
}

bool NtruModel::internalKeygen() {
    SecureChaChaRng rng;
    for (int tries = 0; tries < 2000; ++tries) {
        genTernary(G_F, [&rng](){ return rng(); });   // ✅ теперь std::function
        genTernary(G_G, [&rng](){ return rng(); });

        Poly inv2(G_N, 0);
        if (!invertMod2(G_F, inv2)) continue;
        Poly Finv = henselLiftToQ(G_F, inv2);
        G_H = mulModQ(Finv, G_G);
        return true;
    }
    return false;
}

// ===== H_e_small =====
NtruModel::EHash NtruModel::H_e_small(const Poly& z_modq, const QByteArray& msg) const {
    QByteArray seed;
    const QByteArray tag("NTRUSIGN-H_e_small:v1");
    auto put_u32 = [&](quint32 v){
        char b[4];
        b[0]= static_cast<char>(v>>24);
        b[1]= static_cast<char>(v>>16);
        b[2]= static_cast<char>(v>>8);
        b[3]= static_cast<char>(v);
        seed.append(b,4);
    };
    seed.append(tag);
    put_u32(static_cast<quint32>(G_N));
    put_u32(static_cast<quint32>(G_Q));
    put_u32(static_cast<quint32>(G_ALPHA));
    for (int i=0;i<G_N;++i) {
        quint16 v = static_cast<quint16>(z_modq[i]);
        char b[2]; b[0]= static_cast<char>(v & 0xFF); b[1]= static_cast<char>(v >> 8);
        seed.append(b,2);
    }
    seed.append(msg);

    const quint32 M = static_cast<quint32>(2*G_ALPHA + 1);
    const quint32 L = (65536u / M) * M;

    QByteArray stream;
    stream.resize(G_N * 2 * 3);
    xofFill(seed, reinterpret_cast<uchar*>(stream.data()), static_cast<size_t>(stream.size()));
    int p = 0;
    auto next16 = [&]()->quint16 {
        if (p + 2 > stream.size()) {
            const int old = stream.size();
            stream.resize(old + G_N*2);
            xofFill(seed, reinterpret_cast<uchar*>(stream.data() + old), static_cast<size_t>(stream.size()-old));
        }
        quint16 v = static_cast<quint16>(static_cast<unsigned char>(stream[p])) |
                    (static_cast<quint16>(static_cast<unsigned char>(stream[p+1]))<<8);
        p += 2;
        return v;
    };

    Poly e_small(G_N, 0);
    for (int i=0;i<G_N;++i) {
        quint32 t;
        do { t = next16(); } while (t >= L);
        quint32 u = t % M;
        e_small[i] = static_cast<int>(u) - G_ALPHA;
    }
    Poly e_mod(G_N, 0);
    for (int i=0;i<G_N;++i) {
        int m = e_small[i] % G_Q; if (m < 0) m += G_Q; e_mod[i] = m;
    }
    return { e_small, e_mod };
}

// ===== дискретная гауссиана =====
int NtruModel::sample_gauss_int(const std::function<quint32()>& rng, double sigma) {
    double s = 0.0;
    for (int i=0;i<12;++i) {
        quint32 r = rng();
        double u = static_cast<double>(r) / 4294967296.0;
        s += u;
    }
    double z = (s - 6.0) * sigma;
    long long y = llround(z);
    if (y > (1ll<<31)-1) y = (1ll<<31)-1;
    if (y < -(1ll<<31))  y = -(1ll<<31);
    return static_cast<int>(y);
}

// ===== NTRUSign_once =====
bool NtruModel::NTRUSign_once(const Poly& m, Poly& s_out) const {
    QVector<int> fI(G_N,0), gI(G_N,0), mI(G_N,0);
    for (int i=0;i<G_N;++i) { fI[i]=center(G_F[i], G_Q); gI[i]=center(G_G[i], G_Q); mI[i]=center(m[i], G_Q); }

    QVector<long long> xA(G_N,0), yA(G_N,0);
    for (int i=0;i<G_N;++i) if (mI[i]) {
            for (int j=0;j<G_N;++j) {
                int k = i+j; if (k>=G_N) k-=G_N;
                xA[k] -= static_cast<long long>(mI[i]) * gI[j];
                yA[k] += static_cast<long long>(mI[i]) * fI[j];
            }
        }
    QVector<int> kx(G_N,0), ky(G_N,0);
    for (int i=0;i<G_N;++i) {
        kx[i] = static_cast<int>(llround(static_cast<long double>(xA[i]) / static_cast<long double>(G_Q)));
        ky[i] = static_cast<int>(llround(static_cast<long double>(yA[i]) / static_cast<long double>(G_Q)));
    }
    QVector<long long> sA(G_N,0);
    for (int i=0;i<G_N;++i) {
        if (kx[i]) for (int j=0;j<G_N;++j) { int k=i+j; if (k>=G_N) k-=G_N; sA[k]+= static_cast<long long>(kx[i]) * fI[j]; }
        if (ky[i]) for (int j=0;j<G_N;++j) { int k=i+j; if (k>=G_N) k-=G_N; sA[k]+= static_cast<long long>(ky[i]) * gI[j]; }
    }
    s_out = Poly(G_N,0);
    for (int i=0;i<G_N;++i) s_out[i] = static_cast<int>(sA[i]);

    Poly sMod(G_N,0);
    for (int i=0;i<G_N;++i) sMod[i] = modQ(s_out[i], G_Q);
    Poly sh = mulModQ(sMod, G_H);
    QVector<int> tI(G_N,0);
    for (int i=0;i<G_N;++i) tI[i] = center(modQ(static_cast<long long>(sh[i]) - m[i], G_Q), G_Q);

    long double s2 = 0, t2 = 0;
    for (int i=0;i<G_N;++i) { s2 += static_cast<long double>(s_out[i]) * s_out[i];
        t2 += static_cast<long double>(tI[i])     * tI[i]; }
    long double norm2 = s2 + (G_NU * G_NU) * t2;
    return (norm2 <= static_cast<long double>(G_NORM_BOUND) * static_cast<long double>(G_NORM_BOUND));
}

// ===== sign_strict =====
bool NtruModel::sign_strict(const QByteArray& msg, Signature& sig) const {
    SecureChaChaRng rng;

    for (int tries = 0; tries < G_MAX_SIGN_ATT; ++tries) {
        // 1) Сэмплируем два гауссовых вектора y1, y2 (в целых координатах и их редукции mod Q)
        Poly y1I(G_N, 0), y2I(G_N, 0);
        for (int i = 0; i < G_N; ++i) {
            y1I[i] = sample_gauss_int([&rng]() { return rng(); }, static_cast<double>(G_SIGMA));
            y2I[i] = sample_gauss_int([&rng]() { return rng(); }, static_cast<double>(G_SIGMA));
        }
        Poly y1(G_N, 0), y2(G_N, 0);
        for (int i = 0; i < G_N; ++i) {
            y1[i] = modQ(y1I[i], G_Q);
            y2[i] = modQ(y2I[i], G_Q);
        }

        // 2) Считаем z = y2 - H*y1 и детерминированный e из H_e_small(z, msg)
        Poly hy1 = mulModQ(G_H, y1);
        Poly z   = subMod(y2, hy1);
        EHash eh = H_e_small(z, msg);  // eh.e_small ∈ [−ALPHA..ALPHA], eh.e_mod ≡ eh.e_small (mod Q)

        // 3) Одна итерация NTRUSign (Babai) для e_mod -> sI
        Poly sI;
        if (!NTRUSign_once(eh.e_mod, sI)) {
            continue; // не прошла норма — пробуем снова
        }

        // 4) tI = center(H*sI - e) (в целых представлениях)
        Poly sMod(G_N, 0);
        for (int i = 0; i < G_N; ++i) sMod[i] = modQ(sI[i], G_Q);
        Poly sh = mulModQ(sMod, G_H);

        QVector<int> tI(G_N, 0);
        for (int i = 0; i < G_N; ++i) {
            tI[i] = center(modQ(static_cast<long long>(sh[i]) - eh.e_mod[i], G_Q), G_Q);
        }

        // 5) Собираем подписьные компоненты x1, x2 (mod Q)
        Poly x1(G_N, 0), x2(G_N, 0);
        for (int i = 0; i < G_N; ++i) {
            const int xi1 = y1I[i] - sI[i];
            const int xi2 = y2I[i] - tI[i] - eh.e_small[i];
            x1[i] = modQ(xi1, G_Q);
            x2[i] = modQ(xi2, G_Q);
        }

        // 6) Acceptance-rejection (как в оригинале): exp((<x, -v> - ||v||^2/2)/sigma^2) / MACC
        long double sigma2 = static_cast<long double>(G_SIGMA) * static_cast<long double>(G_SIGMA);
        long double dot = 0.0L, v2 = 0.0L, xnorm2 = 0.0L;

        for (int i = 0; i < G_N; ++i) {
            const int xv1 = center(x1[i], G_Q);
            const int xv2 = center(x2[i], G_Q);
            const int vv1 = -sI[i];
            const int vv2 = -tI[i] - eh.e_small[i];

            dot    += static_cast<long double>(xv1) * vv1 + static_cast<long double>(xv2) * vv2;
            v2     += static_cast<long double>(vv1) * vv1 + static_cast<long double>(vv2) * vv2;
            xnorm2 += static_cast<long double>(xv1) * xv1 + static_cast<long double>(xv2) * xv2;
        }

        long double exponent = (dot - 0.5L * v2) / sigma2;
        if (exponent > 700.0L)  exponent = 700.0L;        // защита от overflow expl()
        if (exponent < -700.0L) exponent = -700.0L;

        const long double R = expl(exponent);
        long double p = R / G_MACC;
        if (p > 1.0L) p = 1.0L;
        if (!std::isfinite(static_cast<double>(p)) || p != p) p = 0.0L;

        const quint32 r = rng();
        const long double U = static_cast<long double>(r) / 4294967296.0L;
        if (U > p) {
            continue; // отклоняем, повторяем
        }

        // 7) Дополнительное ограничение на норму (анти-утечки)
        const long double bound =
                static_cast<long double>(G_ETA) *
                static_cast<long double>(G_SIGMA) *
                sqrtl(2.0L * static_cast<long double>(G_N));

        if (sqrtl(xnorm2) > bound) {
            continue; // превышение безопасной нормы
        }

        // 8) Успех: заполняем результат
        sig.x1 = x1;
        sig.x2 = x2;
        sig.e  = eh.e_mod;
        return true;
    }

    // слишком много попыток — не удалось сгенерировать подпись
    return false;
}


// ===== файловые подписи =====
bool NtruModel::write_sig(const QString& sigPathIn, const Signature& S) const {
    QString sigPath = sigPathIn;
    if (isDirectoryLike(sigPath)) {
        QFileInfo fi(sigPath);
        const QString base = QStringLiteral("signature.sig");
        sigPath = QDir(fi.absoluteFilePath()).filePath(base);
    } else {
        QFileInfo fi(sigPath);
        if (fi.suffix().isEmpty()) sigPath = fi.absolutePath() + "/" + fi.completeBaseName() + ".sig";
    }

    if (!ensureParentDirs(sigPath)) return false;

    QFile file(sigPath);
    if (!file.open(QIODevice::WriteOnly)) return false;

    // формат: "SGN2" + три поли по G_N * uint16
    file.write("SGN2", 4);
    auto write_poly_u16 = [&](const Poly& P){
        for (int i=0;i<G_N;++i) {
            quint16 v = static_cast<quint16>(P[i]);
            file.write(reinterpret_cast<const char*>(&v), sizeof(v));
        }
    };
    write_poly_u16(S.x1); write_poly_u16(S.x2); write_poly_u16(S.e);
    file.close();
    return true;
}

bool NtruModel::read_sig(const QString& sigPath, Signature& S) const {
    QFile file(sigPath);
    if (!file.open(QIODevice::ReadOnly)) return false;
    QByteArray magic = file.read(4);
    if (magic != "SGN2") return false;

    S.x1 = Poly(G_N,0); S.x2 = Poly(G_N,0); S.e = Poly(G_N,0);
    auto read_poly_u16 = [&](Poly& P){
        for (int i=0;i<G_N;++i) {
            quint16 v=0;
            if (file.read(reinterpret_cast<char*>(&v), sizeof(v)) != sizeof(v)) return false;
            P[i] = static_cast<int>(v);
        }
        return true;
    };
    if (!read_poly_u16(S.x1)) return false;
    if (!read_poly_u16(S.x2)) return false;
    if (!read_poly_u16(S.e))  return false;

    const qsizetype expect = 4 + static_cast<qsizetype>(3 * G_N * 2);
    if (file.size() != expect) return false;
    return true;
}

// ===== helpers путей =====
QString NtruModel::toTargetFilePath(const QString& userPath, const QString& defaultName, const QString& defaultExt) {
    QFileInfo fi(userPath);
    if (isDirectoryLike(userPath)) {
        return QDir(fi.absoluteFilePath()).filePath(defaultName);
    } else {
        if (fi.fileName().isEmpty()) return QDir(fi.absolutePath()).filePath(defaultName);
        if (fi.suffix().isEmpty())  return fi.absolutePath() + "/" + fi.completeBaseName() + defaultExt;
        return fi.absoluteFilePath();
    }
}
bool NtruModel::ensureParentDirs(const QString& fullPath) {
    QFileInfo fi(fullPath);
    QDir dir(fi.absolutePath());
    if (dir.exists()) return true;
    return dir.mkpath(".");
}
bool NtruModel::isDirectoryLike(const QString& path) {
    QFileInfo fi(path);
    if (fi.exists()) return fi.isDir();
    if (!path.isEmpty()) {
        const QChar c = path.back();
        return c == '/' || c == '\\';
    }
    return false;
}

// ===== Параметры =====
bool NtruModel::loadParameters(const QString& paramPath) {
    QFile f(paramPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QTextStream in(&f);

    int have = 0;
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) continue;
        const int eq = line.indexOf('=');
        if (eq < 0) continue;
        const QString k = line.left(eq).trimmed();
        const QString v = line.mid(eq+1).trimmed();

        if      (k=="N")            { G_N = v.toInt();          have++; }
        else if (k=="Q")            { G_Q = v.toInt();          have++; }
        else if (k=="D")            { G_D = v.toInt();          have++; }
        else if (k=="NU")           { G_NU = v.toDouble();      have++; }
        else if (k=="NORM_BOUND")   { G_NORM_BOUND = v.toInt(); have++; }
        else if (k=="ETA")          { G_ETA = v.toDouble();     have++; }
        else if (k=="ALPHA")        { G_ALPHA = v.toInt();      have++; }
        else if (k=="SIGMA")        { G_SIGMA = v.toInt();      have++; }
        else if (k=="MAX_SIGN_ATTEMPTS_MASK") { G_MAX_SIGN_ATT = v.toInt(); }
    }
    f.close();

    if (have < 8) return false;
    if (G_N<=0 || G_Q<=0 || G_D<=0 || G_SIGMA<=0 || G_ALPHA<0) return false;

    G_MACC = std::exp(1.0 + 1.0 / (2.0 * static_cast<double>(G_ALPHA) * static_cast<double>(G_ALPHA)));

    G_F = Poly(G_N,0); G_G = Poly(G_N,0); G_H = Poly(G_N,0);
    return true;
}

// ===== Ключи =====
bool NtruModel::keygen() {
    return internalKeygen();
}

bool NtruModel::savePrivateKey(const QString& userPath) {
    const QString path = toTargetFilePath(userPath, "private.key", ".key");
    if (!ensureParentDirs(path)) return false;
    QFile out(path);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream ts(&out);
    ts << "PRIV1\n" << G_N << "\n";
    for (int i=0;i<G_N;++i) { ts << G_F[i]; if (i+1<G_N) ts << ' '; } ts << "\n";
    for (int i=0;i<G_N;++i) { ts << G_G[i]; if (i+1<G_N) ts << ' '; } ts << "\n";
    out.close();
    return true;
}
bool NtruModel::savePublicKey(const QString& userPath) {
    const QString path = toTargetFilePath(userPath, "public.key", ".key");
    if (!ensureParentDirs(path)) return false;
    QFile out(path);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream ts(&out);
    ts << "PUB1\n" << G_N << "\n";
    for (int i=0;i<G_N;++i) { ts << G_H[i]; if (i+1<G_N) ts << ' '; }
    ts << "\n";
    out.close();
    return true;
}
bool NtruModel::loadPrivateKey(const QString& path) {
    QFile in(path);
    if (!in.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QTextStream ts(&in);
    QString hdr; ts >> hdr; if (hdr != "PRIV1") return false;
    int n; ts >> n; if (n != G_N) return false;
    G_F = Poly(G_N,0); G_G = Poly(G_N,0);
    for (int i=0;i<G_N;++i) { long long v; ts >> v; G_F[i] = modQ(v, G_Q); }
    for (int i=0;i<G_N;++i) { long long v; ts >> v; G_G[i] = modQ(v, G_Q); }
    in.close();

    Poly inv2(G_N,0);
    if (!invertMod2(G_F, inv2)) return false;
    Poly Finv = henselLiftToQ(G_F, inv2);
    G_H = mulModQ(Finv, G_G);
    return true;
}
bool NtruModel::loadPublicKey(const QString& path) {
    QFile in(path);
    if (!in.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QTextStream ts(&in);
    QString hdr; ts >> hdr; if (hdr != "PUB1") return false;
    int n; ts >> n; if (n != G_N) return false;
    G_H = Poly(G_N,0);
    for (int i=0;i<G_N;++i) { long long v; ts >> v; G_H[i] = modQ(v, G_Q); }
    in.close();
    return true;
}

// ===== Подпись/Проверка (файлы) =====
bool NtruModel::signFile(const QString& docPath, const QString& sigPlace) {
    QFile in(docPath);
    if (!in.open(QIODevice::ReadOnly)) return false;
    QByteArray msg = in.readAll();
    in.close();

    Signature S;
    if (!sign_strict(msg, S)) return false;
    return write_sig(sigPlace, S);
}

bool NtruModel::verifyFile(const QString& docPath, const QString& sigPath) {
    QFile in(docPath);
    if (!in.open(QIODevice::ReadOnly)) return false;
    QByteArray msg = in.readAll();
    in.close();

    Signature S;
    if (!read_sig(sigPath, S)) return false;

    // hx1 = H * x1 ; z = x2 - hx1
    Poly hx1 = mulModQ(G_H, S.x1);
    Poly z   = subMod(S.x2, hx1);
    EHash eh2 = H_e_small(z, msg);

    for (int i=0;i<G_N;++i) if (eh2.e_mod[i] != S.e[i]) return false;

    long double x2norm = 0;
    for (int i=0;i<G_N;++i) {
        int a = center(S.x1[i], G_Q);
        int b = center(S.x2[i], G_Q);
        x2norm += static_cast<long double>(a) * a + static_cast<long double>(b) * b;
    }
    long double bound = static_cast<long double>(G_ETA) * static_cast<long double>(G_SIGMA) * sqrtl(2.0L * static_cast<long double>(G_N));
    if (sqrtl(x2norm) > bound) return false;

    return true;
}
