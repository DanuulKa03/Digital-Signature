
#pragma once
#include <QtCore>
#include <QtNetwork>
#include "Encryptor.h"

class FileReceiver : public QObject {
    Q_OBJECT
public:
    explicit FileReceiver(const QByteArray& password, QObject* parent=nullptr)
        : QObject(parent), m_password(password) {}

    void setOutputDir(const QString& dir) { m_outDir = dir; }

    // Обработать одно соединение (блокирующе для простоты)
    bool handle(QTcpSocket* sock, QString* savedPathOut=nullptr, QString* errOut=nullptr) {
        QByteArray header;
        // Читаем до \r\n\r\n
        while (true) {
            if (!sock->waitForReadyRead(5000)) { if (errOut) *errOut = "Таймаут чтения заголовка"; return false; }
            header += sock->readAll();
            int idx = header.indexOf("\r\n\r\n");
            if (idx >= 0) { header = header.left(idx); break; }
            if (header.size() > (1024*1024)) { if (errOut) *errOut = "Слишком длинный заголовок"; return false; }
        }

        QJsonParseError pe{};
        QJsonDocument doc = QJsonDocument::fromJson(header, &pe);
        if (pe.error != QJsonParseError::NoError || !doc.isObject()) {
            if (errOut) *errOut = "Некорректный JSON заголовок";
            return false;
        }
        QJsonObject head = doc.object();
        QString filename = head.value("filename").toString("received.bin");
        qint64 size = head.value("size").toInteger(0);
        int tagLen = head.value("tag_len").toInt(16);
        QByteArray salt = QByteArray::fromBase64(head.value("salt").toString().toLatin1());
        QByteArray iv = QByteArray::fromBase64(head.value("iv").toString().toLatin1());

        if (size <= 0 || tagLen <= 0) {
            if (errOut) *errOut = "Некорректные поля size/tag_len";
            return false;
        }

        QByteArray payload;
        payload.resize(size);
        qint64 got = 0;
        while (got < size) {
            if (!sock->waitForReadyRead(5000)) { if (errOut) *errOut = "Таймаут чтения полезной нагрузки"; return false; }
            QByteArray chunk = sock->read(size - got);
            if (chunk.isEmpty()) continue;
            memcpy(payload.data() + got, chunk.constData(), chunk.size());
            got += chunk.size();
        }

        QByteArray cipher = payload.left(size - tagLen);
        QByteArray tag = payload.right(tagLen);
        QDir().mkpath(m_outDir);
        QString outPath = QDir(m_outDir).filePath(filename);

        QString derr;
        bool ok = Encryptor::decryptToFileAesGcm(outPath, m_password, salt, iv, cipher, tag, &derr);
        if (!ok) { if (errOut) *errOut = derr; return false; }

        if (savedPathOut) *savedPathOut = outPath;
        return true;
    }

private:
    QByteArray m_password;
    QString m_outDir = QStringLiteral("/data/out");
};
