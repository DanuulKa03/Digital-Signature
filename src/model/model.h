#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QVector>
#include <QByteArray>
#include <QFile>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <cmath>

class Model : public QObject
{
Q_OBJECT

public:
    explicit Model(QObject *parent = nullptr);

    struct Signature {
        QVector<int> x1;
        QVector<int> x2;
        QVector<int> e;

        Signature() = default;
        Signature(const Signature& other) = default;
        Signature& operator=(const Signature& other) = default;
    };

    struct KeyPair {
        QVector<int> f;
        QVector<int> g;
        QVector<int> h;

        KeyPair() = default;
        KeyPair(const KeyPair& other) = default;
        KeyPair& operator=(const KeyPair& other) = default;
    };

    // Main operations
    bool loadParameters(const QString& filePath);
    bool generateKeys();
    bool signFile(const QString& filePath, Signature& signature);
    bool verifySignature(const QString& filePath, const Signature& signature);

    // Getters
    const KeyPair& getKeyPair() const { return m_keyPair; }
    QString getLastError() const { return m_lastError; }

    // Save/load keys
    bool savePrivateKey(const QString& filePath);
    bool savePublicKey(const QString& filePath);
    bool loadPrivateKey(const QString& filePath);
    bool loadPublicKey(const QString& filePath);

    // Save/load signature
    bool saveSignature(const QString& filePath, const Signature& signature);
    bool loadSignature(const QString& filePath, Signature& signature);

signals:
    void operationCompleted(const QString& message);
    void errorOccurred(const QString& error);

private:
    // Parameters
    int m_N = 0;
    int m_Q = 0;
    int m_D = 0;
    int m_normBound = 0;
    int m_alpha = 0;
    int m_sigma = 0;
    double m_nu = 0.0;
    double m_eta = 0.0;
    double m_macc = 0.0;

    int m_maxSignAttempts = 1000;

    KeyPair m_keyPair;
    QString m_lastError;

    // Arithmetic operations
    int modQ(long long x) const;
    int center(int a) const;
    QVector<int> zeroPoly() const;
    QVector<int> subMod(const QVector<int>& a, const QVector<int>& b) const;
    QVector<int> mulModQ(const QVector<int>& a, const QVector<int>& b) const;
    QVector<int> mulModPow2(const QVector<int>& a, const QVector<int>& b, int M) const;

    // Key operations
    bool invertMod2(const QVector<int>& f, QVector<int>& inv2) const;
    QVector<int> henselLiftToQ(const QVector<int>& f, const QVector<int>& inv2) const;
    void genTernary(QVector<int>& a, QRandomGenerator& rng) const;

    // Hashing and randomness
    QVector<int> H_e_small(const QVector<int>& z_modq, const QByteArray& msg) const;
    int sampleGaussInt(QRandomGenerator& rng, double sigma) const;

    // Signing
    bool NTRUSign_once(const QVector<int>& m, QVector<int>& s_out) const;

    // Helper functions
    QByteArray calculateFileHash(const QString& filePath) const;
    bool validateParameters();
};

#endif // MODEL_H