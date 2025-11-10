
#pragma once
#include <QtCore>
#include <openssl/evp.h>
#include <openssl/rand.h>

class Encryptor : public QObject {
    Q_OBJECT
public:
    explicit Encryptor(QObject* parent=nullptr) : QObject(parent) {}

    struct EncResult {
        QByteArray salt;    // 16 bytes
        QByteArray iv;      // 12 bytes
        QByteArray cipher;  // ciphertext
        QByteArray tag;     // 16 bytes
        QString error;
        bool ok() const { return error.isEmpty(); }
    };

    // PBKDF2-HMAC-SHA256 -> 32-byte key
    static bool deriveKey(const QByteArray& password, const QByteArray& salt, QByteArray& keyOut, int iterations=200000) {
        keyOut.resize(32);
        int r = PKCS5_PBKDF2_HMAC(password.constData(), password.size(),
                                  reinterpret_cast<const unsigned char*>(salt.constData()), salt.size(),
                                  iterations, EVP_sha256(), keyOut.size(),
                                  reinterpret_cast<unsigned char*>(keyOut.data()));
        return r == 1;
    }

    static EncResult encryptFileAesGcm(const QString& path, const QByteArray& password) {
        EncResult res;
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) {
            res.error = QStringLiteral("Не удалось открыть файл: %1").arg(path);
            return res;
        }
        QByteArray plain = f.readAll();
        f.close();

        res.salt.resize(16);
        res.iv.resize(12);
        if (RAND_bytes(reinterpret_cast<unsigned char*>(res.salt.data()), res.salt.size()) != 1 ||
            RAND_bytes(reinterpret_cast<unsigned char*>(res.iv.data()), res.iv.size()) != 1) {
            res.error = QStringLiteral("RAND_bytes() ошибка");
            return res;
        }

        QByteArray key;
        if (!deriveKey(password, res.salt, key)) {
            res.error = QStringLiteral("PBKDF2 ошибка");
            return res;
        }

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) { res.error = "EVP_CIPHER_CTX_new() fail"; return res; }

        int ok = EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr);
        if (ok != 1) { EVP_CIPHER_CTX_free(ctx); res.error = "EncryptInit fail"; return res; }
        ok = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, res.iv.size(), nullptr);
        if (ok != 1) { EVP_CIPHER_CTX_free(ctx); res.error = "SET_IVLEN fail"; return res; }
        ok = EVP_EncryptInit_ex(ctx, nullptr, nullptr,
            reinterpret_cast<const unsigned char*>(key.constData()),
            reinterpret_cast<const unsigned char*>(res.iv.constData()));
        if (ok != 1) { EVP_CIPHER_CTX_free(ctx); res.error = "EncryptInit key/iv fail"; return res; }

        res.cipher.resize(plain.size());
        int outlen1 = 0, outlen2 = 0;
        ok = EVP_EncryptUpdate(ctx,
            reinterpret_cast<unsigned char*>(res.cipher.data()), &outlen1,
            reinterpret_cast<const unsigned char*>(plain.constData()), plain.size());
        if (ok != 1) { EVP_CIPHER_CTX_free(ctx); res.error = "EncryptUpdate fail"; return res; }

        ok = EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(res.cipher.data()) + outlen1, &outlen2);
        if (ok != 1) { EVP_CIPHER_CTX_free(ctx); res.error = "EncryptFinal fail"; return res; }
        res.cipher.resize(outlen1 + outlen2);

        res.tag.resize(16);
        ok = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, res.tag.size(), res.tag.data());
        EVP_CIPHER_CTX_free(ctx);
        if (ok != 1) { res.error = "GET_TAG fail"; return res; }

        return res;
    }

    static bool decryptToFileAesGcm(const QString& outPath, const QByteArray& password,
                                    const QByteArray& salt, const QByteArray& iv,
                                    const QByteArray& cipher, const QByteArray& tag,
                                    QString* errOut=nullptr) {
        QByteArray key;
        if (!deriveKey(password, salt, key)) {
            if (errOut) *errOut = QStringLiteral("PBKDF2 ошибка");
            return false;
        }

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) { if (errOut) *errOut = "CTX_new fail"; return false; }

        int ok = EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr);
        if (ok != 1) { if (errOut) *errOut = "DecryptInit fail"; EVP_CIPHER_CTX_free(ctx); return false; }
        ok = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv.size(), nullptr);
        if (ok != 1) { if (errOut) *errOut = "SET_IVLEN fail"; EVP_CIPHER_CTX_free(ctx); return false; }
        ok = EVP_DecryptInit_ex(ctx, nullptr, nullptr,
            reinterpret_cast<const unsigned char*>(key.constData()),
            reinterpret_cast<const unsigned char*>(iv.constData()));
        if (ok != 1) { if (errOut) *errOut = "DecryptInit key/iv fail"; EVP_CIPHER_CTX_free(ctx); return false; }

        QByteArray plain;
        plain.resize(cipher.size());
        int outlen1 = 0, outlen2 = 0;
        ok = EVP_DecryptUpdate(ctx,
            reinterpret_cast<unsigned char*>(plain.data()), &outlen1,
            reinterpret_cast<const unsigned char*>(cipher.constData()), cipher.size());
        if (ok != 1) { if (errOut) *errOut = "DecryptUpdate fail"; EVP_CIPHER_CTX_free(ctx); return false; }

        // set tag BEFORE final
        ok = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tag.size(), const_cast<char*>(tag.constData()));
        if (ok != 1) { if (errOut) *errOut = "SET_TAG fail"; EVP_CIPHER_CTX_free(ctx); return false; }

        ok = EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(plain.data()) + outlen1, &outlen2);
        if (ok != 1) { if (errOut) *errOut = "DecryptFinal (auth fail)"; EVP_CIPHER_CTX_free(ctx); return false; }
        plain.resize(outlen1 + outlen2);
        EVP_CIPHER_CTX_free(ctx);

        QFile out(outPath);
        if (!out.open(QIODevice::WriteOnly)) {
            if (errOut) *errOut = QStringLiteral("Не удалось открыть для записи: %1").arg(outPath);
            return false;
        }
        out.write(plain);
        out.close();
        return true;
    }
};
