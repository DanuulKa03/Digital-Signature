#pragma once
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QByteArray>
#include <QtCore/QPair>
#include <functional>


class NtruModel {
public:
    NtruModel();

    // параметры
    bool loadParameters(const QString& paramPath);

    // ключи
    bool keygen();
    bool savePrivateKey(const QString& userPath);
    bool savePublicKey (const QString& userPath);
    bool loadPrivateKey(const QString& path);
    bool loadPublicKey (const QString& path);

    // подпись/проверка (работа с файлами)
    bool signFile  (const QString& docPath, const QString& sigPlace);
    bool verifyFile(const QString& docPath, const QString& sigPath);

private:
    // типы
    using Poly = QVector<int>;
    struct EHash { Poly e_small; Poly e_mod; };
    struct Signature { Poly x1, x2, e; };

    // глобальные параметры (из файла)
    int G_N = 0;
    int G_Q = 0;
    int G_D = 0;
    int G_NORM_BOUND = 0;
    int G_ALPHA = 0;
    int G_SIGMA = 0;
    int G_MAX_SIGN_ATT = 1000;
    double G_NU = 0.0;
    double G_ETA = 0.0;
    double G_MACC = 0.0;

    // ключи
    Poly G_F, G_G, G_H;

    // утилиты
    static int modQ(long long x, int q);
    static int center(int a, int q);
    Poly zeroP() const;
    Poly subMod(const Poly& A, const Poly& B) const;
    Poly mulModQ(const Poly& A, const Poly& B) const;
    Poly mulModPow2(const Poly& A, const Poly& B, int M) const;

    // GF(2) + Хензель
    struct Poly2 { QVector<unsigned char> a; };
    static int deg2(const Poly2& p);
    static Poly2 trim2(const Poly2& p);
    static Poly2 add2 (const Poly2& A, const Poly2& B);
    static Poly2 shl2_nonCirc(const Poly2& A, int k);
    static Poly2 mul2_nonCirc(const Poly2& A, const Poly2& B);
    static void  div2_poly(const Poly2& A, const Poly2& B, Poly2& Q, Poly2& R);
    bool invertMod2(const Poly& f, Poly& inv2_out) const;
    Poly henselLiftToQ(const Poly& f, const Poly& inv2) const;

    // генерация ключей
    void genTernary(Poly& a, const std::function<quint32()>& rng);
    bool internalKeygen();

    // RNG (простой интерфейс на chacha20 из исходника — оставим как есть)
    struct ChaCha20 {
        quint32 st[16];
        static inline quint32 rotl(quint32 x, int n) { return (x << n) | (x >> (32 - n)); }
        static inline void qr(quint32 s[16], int a, int b, int c, int d) {
            s[a]+=s[b]; s[d]^=s[a]; s[d]=rotl(s[d],16);
            s[c]+=s[d]; s[b]^=s[c]; s[b]=rotl(s[b],12);
            s[a]+=s[b]; s[d]^=s[a]; s[d]=rotl(s[d],8);
            s[c]+=s[d]; s[b]^=s[c]; s[b]=rotl(s[b],7);
        }
        void set(const uchar key[32], const uchar nonce[12], quint32 counter = 1);
        void block(uchar out[64]);
    };
    struct SecureChaChaRng {
        ChaCha20 ch; uchar buf[64]; int idx = 64;
        SecureChaChaRng();
        void reseed(const void* extra, size_t elen);
        quint32 operator()();
    };

    // хэши и XOF через QCryptographicHash
    static QByteArray sha256(const QByteArray& data);
    static void xofFill(const QByteArray& seed, uchar* out, size_t len);

    // H_e_small
    EHash H_e_small(const Poly& z_modq, const QByteArray& msg) const;

    // дискретная "гауссиана" (CRN + 12 сумм)
    static int sample_gauss_int(const std::function<quint32()>& rng, double sigma);
    // NTRUSign_once
    bool NTRUSign_once(const Poly& m, Poly& s_out) const;

    // sign/verify core
    bool sign_strict(const QByteArray& msg, Signature& sig) const;

    // файловые операции подписи
    bool write_sig(const QString& sigPath, const Signature& S) const;
    bool read_sig(const QString& sigPath, Signature& S) const;

    // хелперы путей
    static QString toTargetFilePath(const QString& userPath, const QString& defaultName, const QString& defaultExt);
    static bool ensureParentDirs(const QString& fullPath);
    static bool isDirectoryLike(const QString& path);
};
