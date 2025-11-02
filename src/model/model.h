#ifndef NTRU_MODEL_H
#define NTRU_MODEL_H

#include <vector>
#include <QString>

// Data type for polynomial coefficients (use 64-bit to handle mod q up to ~2^33 safely)
using Coeff = long long;

/**
 * Structure to hold NTRUSign security parameters.
 */
struct NtruParams {
    int N;          // Polynomial degree (dimension)
    Coeff q;        // Modulus
    int d;          // Number of +1 (and -1) for ternary polynomials
    int perturbCount; // ν: number of perturbation vectors
};

/**
 * Structure for NTRUSign private key (f, g, and perturbation polynomials).
 */
struct NtruPrivKey {
    std::vector<Coeff> f;                      // private polynomial f (length N)
    std::vector<Coeff> g;                      // private polynomial g (length N) such that f*h ≡ g (mod q)
    std::vector< std::vector<Coeff> > eList;   // perturbation polynomials e_i (length N each)
};

/**
 * Structure for NTRUSign public key (h polynomial).
 */
struct NtruPubKey {
    std::vector<Coeff> h;  // public polynomial h (length N) = f^(-1)*g mod q
};

/**
 * The Model class provides static methods to perform NTRUSign key generation, signing, and verification.
 */
class NtruModel {
public:
    // Load parameters from a text file.
    static bool loadParameters(const QString &paramFile, NtruParams &params);

    // Key generation: generate NTRUSign key pair given security parameters.
    static bool generateKeyPair(const NtruParams &params, NtruPrivKey &privKey, NtruPubKey &pubKey);

    // Save keys to files (public and private key files).
    static bool savePrivateKey(const QString &privKeyFile, const NtruParams &params, const NtruPrivKey &privKey);
    static bool savePublicKey(const QString &pubKeyFile, const NtruParams &params, const NtruPubKey &pubKey);

    // Load keys from files.
    static bool loadPrivateKey(const QString &privKeyFile, const NtruParams &params, NtruPrivKey &privKey);
    static bool loadPublicKey(const QString &pubKeyFile, const NtruParams &params, NtruPubKey &pubKey);

    // Sign a message file using the private key and parameters, output signature polynomial to file.
    static bool signMessage(const NtruParams &params, const NtruPrivKey &privKey,
                             const QString &messageFile, const QString &signatureFile);

    // Verify a message's signature using the public key and parameters.
    static bool verifyMessage(const NtruParams &params, const NtruPubKey &pubKey,
                               const QString &messageFile, const QString &signatureFile);

private:
    // Polynomial arithmetic helpers (coefficients mod q, polynomials mod X^N - 1):
    static void polyAdd(std::vector<Coeff> &res, const std::vector<Coeff> &a, const std::vector<Coeff> &b, Coeff q);
    static void polySub(std::vector<Coeff> &res, const std::vector<Coeff> &a, const std::vector<Coeff> &b, Coeff q);
    static void polyMulMod(std::vector<Coeff> &res, const std::vector<Coeff> &a, const std::vector<Coeff> &b, const NtruParams &params);
    static bool invertPolyMod(const std::vector<Coeff> &a, const NtruParams &params, std::vector<Coeff> &a_inv);

    // Utility to reduce polynomial coefficients into range [-(q/2), q/2].
    static void centerPoly(std::vector<Coeff> &poly, Coeff q);
};

#endif // NTRU_MODEL_H
