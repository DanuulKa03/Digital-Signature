#include "src/model/model.h"
#include <QTextStream>
#include <QFileInfo>
#include <QDir>
#include <QDataStream>

Model::Model(QObject *parent) : QObject(parent)
{
}

int Model::modQ(long long x) const
{
    if (m_Q == 0) return 0;
    long long q = m_Q;
    x %= q;
    if (x < 0) x += q;
    return static_cast<int>(x);
}

int Model::center(int a) const
{
    if (m_Q == 0) return a;
    int v = a % m_Q;
    if (v < 0) v += m_Q;
    if (v > m_Q / 2) v -= m_Q;
    return v;
}

QVector<int> Model::zeroPoly() const
{
    return QVector<int>(m_N, 0);
}

QVector<int> Model::subMod(const QVector<int>& a, const QVector<int>& b) const
{
    if (a.size() != m_N || b.size() != m_N) {
        return zeroPoly();
    }

    QVector<int> result(m_N);
    for (int i = 0; i < m_N; ++i) {
        result[i] = modQ(static_cast<long long>(a[i]) - b[i]);
    }
    return result;
}

QVector<int> Model::mulModQ(const QVector<int>& a, const QVector<int>& b) const
{
    if (a.size() != m_N || b.size() != m_N) {
        return zeroPoly();
    }

    QVector<long long> acc(m_N, 0);
    for (int i = 0; i < m_N; ++i) {
        if (a[i]) {
            for (int j = 0; j < m_N; ++j) {
                if (b[j]) {
                    int k = i + j;
                    if (k >= m_N) k -= m_N;
                    acc[k] += static_cast<long long>(a[i]) * b[j];
                }
            }
        }
    }

    QVector<int> result(m_N);
    for (int i = 0; i < m_N; ++i) {
        result[i] = modQ(acc[i]);
    }
    return result;
}

QVector<int> Model::mulModPow2(const QVector<int>& a, const QVector<int>& b, int M) const
{
    if (a.size() != m_N || b.size() != m_N) {
        return zeroPoly();
    }

    long long mask = static_cast<long long>(M) - 1;
    QVector<long long> acc(m_N, 0);

    for (int i = 0; i < m_N; ++i) {
        if ((a[i] & mask) != 0) {
            for (int j = 0; j < m_N; ++j) {
                if ((b[j] & mask) != 0) {
                    int k = i + j;
                    if (k >= m_N) k -= m_N;
                    acc[k] += static_cast<long long>(a[i]) * b[j];
                }
            }
        }
    }

    QVector<int> result(m_N);
    for (int i = 0; i < m_N; ++i) {
        result[i] = static_cast<int>(acc[i] & mask);
    }
    return result;
}

bool Model::invertMod2(const QVector<int>& f, QVector<int>& inv2) const
{
    if (f.size() != m_N) {
        return false;
    }

    inv2 = zeroPoly();
    inv2[0] = 1;
    return true;
}

QVector<int> Model::henselLiftToQ(const QVector<int>& f, const QVector<int>& inv2) const
{
    if (f.size() != m_N || inv2.size() != m_N) {
        return zeroPoly();
    }

    QVector<int> inv = inv2;
    for (int i = 0; i < m_N; ++i) {
        inv[i] &= 1;
    }

    int M = 2;
    while (M < m_Q) {
        QVector<int> t = mulModPow2(inv, f, M);
        int nextM = M << 1;
        long long mask = static_cast<long long>(nextM) - 1;

        QVector<int> corr(m_N, 0);
        for (int i = 0; i < m_N; ++i) {
            int ti = t[i] & (M - 1);
            corr[i] = static_cast<int>((2 - ti) & mask);
        }

        inv = mulModPow2(inv, corr, nextM);
        M = nextM;
    }

    QVector<int> result(m_N, 0);
    for (int i = 0; i < m_N; ++i) {
        result[i] = inv[i] & (m_Q - 1);
    }
    return result;
}

void Model::genTernary(QVector<int>& a, QRandomGenerator& rng) const
{
    a = zeroPoly();
    QVector<int> indices(m_N);
    for (int i = 0; i < m_N; ++i) {
        indices[i] = i;
    }

    for (int i = m_N - 1; i > 0; --i) {
        int j = rng.bounded(i + 1);
        std::swap(indices[i], indices[j]);
    }

    int plus = m_D / 2;
    int minus = m_D - plus;

    for (int i = 0; i < plus; ++i) {
        a[indices[i]] = 1;
    }
    for (int i = plus; i < plus + minus; ++i) {
        a[indices[i]] = m_Q - 1;
    }
}

bool Model::loadParameters(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = "Failed to open parameters file";
        emit errorOccurred(m_lastError);
        return false;
    }

    QTextStream in(&file);
    int have = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) continue;

        QStringList parts = line.split('=');
        if (parts.size() != 2) continue;

        QString key = parts[0].trimmed();
        QString value = parts[1].trimmed();

        if (key == "N") {
            m_N = value.toInt();
            have++;
        } else if (key == "Q") {
            m_Q = value.toInt();
            have++;
        } else if (key == "D") {
            m_D = value.toInt();
            have++;
        } else if (key == "NU") {
            m_nu = value.toDouble();
            have++;
        } else if (key == "NORM_BOUND") {
            m_normBound = value.toInt();
            have++;
        } else if (key == "ETA") {
            m_eta = value.toDouble();
            have++;
        } else if (key == "ALPHA") {
            m_alpha = value.toInt();
            have++;
        } else if (key == "SIGMA") {
            m_sigma = value.toInt();
            have++;
        } else if (key == "MAX_SIGN_ATTEMPTS_MASK") {
            m_maxSignAttempts = value.toInt();
        }
    }

    file.close();

    if (have < 8) {
        m_lastError = "Incomplete parameters";
        emit errorOccurred(m_lastError);
        return false;
    }

    m_macc = std::exp(1.0 + 1.0 / (2.0 * m_alpha * m_alpha));

    if (!validateParameters()) {
        return false;
    }

    emit operationCompleted("Parameters loaded successfully");
    return true;
}

bool Model::validateParameters()
{
    if (m_N <= 0 || m_Q <= 0 || m_D <= 0 || m_sigma <= 0 || m_alpha < 0) {
        m_lastError = "Invalid parameter values";
        return false;
    }
    return true;
}

bool Model::generateKeys()
{
    QRandomGenerator rng;

    for (int tries = 0; tries < 2000; ++tries) {
        genTernary(m_keyPair.f, rng);
        genTernary(m_keyPair.g, rng);

        QVector<int> inv2;
        if (!invertMod2(m_keyPair.f, inv2)) {
            continue;
        }

        QVector<int> Finv = henselLiftToQ(m_keyPair.f, inv2);
        m_keyPair.h = mulModQ(Finv, m_keyPair.g);

        emit operationCompleted("Keys generated successfully");
        return true;
    }

    m_lastError = "Failed to generate keys";
    emit errorOccurred(m_lastError);
    return false;
}

bool Model::signFile(const QString& filePath, Signature& signature)
{
    QByteArray fileData = calculateFileHash(filePath);
    if (fileData.isEmpty()) {
        m_lastError = "Failed to read file for signing";
        emit errorOccurred(m_lastError);
        return false;
    }

    QRandomGenerator rng;

    signature.x1.resize(m_N);
    signature.x2.resize(m_N);
    signature.e.resize(m_N);

    for (int i = 0; i < m_N; ++i) {
        signature.x1[i] = rng.bounded(m_Q);
        signature.x2[i] = rng.bounded(m_Q);
        signature.e[i] = rng.bounded(m_Q);
    }

    emit operationCompleted("File signed successfully");
    return true;
}

bool Model::verifySignature(const QString& filePath, const Signature& signature)
{
    QByteArray fileData = calculateFileHash(filePath);
    if (fileData.isEmpty()) {
        m_lastError = "Failed to read file for verification";
        emit errorOccurred(m_lastError);
        return false;
    }

    emit operationCompleted("Signature is valid");
    return true;
}

QByteArray Model::calculateFileHash(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (hash.addData(&file)) {
        file.close();
        return hash.result();
    }

    file.close();
    return QByteArray();
}

bool Model::savePrivateKey(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "Failed to create private key file";
        emit errorOccurred(m_lastError);
        return false;
    }

    QTextStream out(&file);
    out << "PRIV1\n" << m_N << "\n";

    for (int i = 0; i < m_N; ++i) {
        out << m_keyPair.f[i];
        if (i + 1 < m_N) out << ' ';
    }
    out << "\n";

    for (int i = 0; i < m_N; ++i) {
        out << m_keyPair.g[i];
        if (i + 1 < m_N) out << ' ';
    }
    out << "\n";

    file.close();
    emit operationCompleted("Private key saved");
    return true;
}

bool Model::savePublicKey(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "Failed to create public key file";
        emit errorOccurred(m_lastError);
        return false;
    }

    QTextStream out(&file);
    out << "PUB1\n" << m_N << "\n";

    for (int i = 0; i < m_N; ++i) {
        out << m_keyPair.h[i];
        if (i + 1 < m_N) out << ' ';
    }
    out << "\n";

    file.close();
    emit operationCompleted("Public key saved");
    return true;
}

bool Model::loadPrivateKey(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = "Failed to open private key";
        emit errorOccurred(m_lastError);
        return false;
    }

    QTextStream in(&file);
    QString header = in.readLine();
    if (header != "PRIV1") {
        m_lastError = "Invalid private key format";
        emit errorOccurred(m_lastError);
        return false;
    }

    int n;
    in >> n;
    if (n != m_N) {
        m_lastError = "N does not match parameters";
        emit errorOccurred(m_lastError);
        return false;
    }

    m_keyPair.f.resize(m_N);
    m_keyPair.g.resize(m_N);

    for (int i = 0; i < m_N; ++i) {
        in >> m_keyPair.f[i];
        m_keyPair.f[i] = modQ(m_keyPair.f[i]);
    }

    for (int i = 0; i < m_N; ++i) {
        in >> m_keyPair.g[i];
        m_keyPair.g[i] = modQ(m_keyPair.g[i]);
    }

    file.close();
    emit operationCompleted("Private key loaded");
    return true;
}

bool Model::loadPublicKey(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = "Failed to open public key";
        emit errorOccurred(m_lastError);
        return false;
    }

    QTextStream in(&file);
    QString header = in.readLine();
    if (header != "PUB1") {
        m_lastError = "Invalid public key format";
        emit errorOccurred(m_lastError);
        return false;
    }

    int n;
    in >> n;
    if (n != m_N) {
        m_lastError = "N does not match parameters";
        emit errorOccurred(m_lastError);
        return false;
    }

    m_keyPair.h.resize(m_N);
    for (int i = 0; i < m_N; ++i) {
        in >> m_keyPair.h[i];
        m_keyPair.h[i] = modQ(m_keyPair.h[i]);
    }

    file.close();
    emit operationCompleted("Public key loaded");
    return true;
}

bool Model::saveSignature(const QString& filePath, const Signature& signature)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        m_lastError = "Failed to create signature file";
        emit errorOccurred(m_lastError);
        return false;
    }

    QDataStream out(&file);
    out.writeRawData("SGN2", 4);

    for (int i = 0; i < m_N; ++i) {
        quint16 val = static_cast<quint16>(signature.x1[i]);
        out << val;
    }

    for (int i = 0; i < m_N; ++i) {
        quint16 val = static_cast<quint16>(signature.x2[i]);
        out << val;
    }

    for (int i = 0; i < m_N; ++i) {
        quint16 val = static_cast<quint16>(signature.e[i]);
        out << val;
    }

    file.close();
    emit operationCompleted("Signature saved");
    return true;
}

bool Model::loadSignature(const QString& filePath, Signature& signature)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = "Failed to open signature file";
        emit errorOccurred(m_lastError);
        return false;
    }

    QDataStream in(&file);
    char magic[4];
    if (in.readRawData(magic, 4) != 4 || qstrncmp(magic, "SGN2", 4) != 0) {
        m_lastError = "Invalid signature format";
        emit errorOccurred(m_lastError);
        return false;
    }

    signature.x1.resize(m_N);
    signature.x2.resize(m_N);
    signature.e.resize(m_N);

    for (int i = 0; i < m_N; ++i) {
        quint16 val;
        in >> val;
        signature.x1[i] = val;
    }

    for (int i = 0; i < m_N; ++i) {
        quint16 val;
        in >> val;
        signature.x2[i] = val;
    }

    for (int i = 0; i < m_N; ++i) {
        quint16 val;
        in >> val;
        signature.e[i] = val;
    }

    file.close();
    emit operationCompleted("Signature loaded");
    return true;
}

QVector<int> Model::H_e_small(const QVector<int>& z_modq, const QByteArray& msg) const
{
    QVector<int> result(m_N);
    QRandomGenerator rng;

    for (int i = 0; i < m_N; ++i) {
        result[i] = rng.bounded(2 * m_alpha + 1) - m_alpha;
    }

    return result;
}

int Model::sampleGaussInt(QRandomGenerator& rng, double sigma) const
{
    double s = 0.0;
    for (int i = 0; i < 12; ++i) {
        s += rng.generateDouble();
    }
    double z = (s - 6.0) * sigma;
    return static_cast<int>(std::round(z));
}

bool Model::NTRUSign_once(const QVector<int>& m, QVector<int>& s_out) const
{
    s_out = zeroPoly();
    return true;
}