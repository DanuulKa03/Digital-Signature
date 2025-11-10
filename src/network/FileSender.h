
#pragma once
#include <QtCore>
#include <QtNetwork>
#include "Encryptor.h"

class FileSender : public QObject {
    Q_OBJECT
public:
    explicit FileSender(const QByteArray& password, QObject* parent=nullptr)
        : QObject(parent), m_password(password) {}

    // remoteHost: IP или DNS, remotePort: порт сервера
    // localPath: путь к исходному файлу, remoteName: имя, под которым сохранить на сервере
    bool send(const QString& remoteHost, quint16 remotePort,
              const QString& localPath, const QString& remoteName, QString* errOut=nullptr) {
        Encryptor::EncResult enc = Encryptor::encryptFileAesGcm(localPath, m_password);
        if (!enc.ok()) {
            if (errOut) *errOut = enc.error;
            return false;
        }

        QTcpSocket sock;
        sock.connectToHost(remoteHost, remotePort);
        if (!sock.waitForConnected(5000)) {
            if (errOut) *errOut = "Не удалось подключиться к серверу";
            return false;
        }

        // Заголовок JSON
        QJsonObject head;
        head["filename"] = remoteName;
        head["size"] = static_cast<qint64>(enc.cipher.size() + enc.tag.size());
        head["salt"] = QString::fromLatin1(enc.salt.toBase64());
        head["iv"] = QString::fromLatin1(enc.iv.toBase64());
        head["tag_len"] = enc.tag.size();
        QByteArray header = QJsonDocument(head).toJson(QJsonDocument::Compact);
        header.append("\r\n\r\n");

        // Отправка
        QByteArray payload;
        payload.reserve(header.size() + enc.cipher.size() + enc.tag.size());
        payload.append(header);
        payload.append(enc.cipher);
        payload.append(enc.tag);

        qint64 written = 0;
        while (written < payload.size()) {
            qint64 w = sock.write(payload.constData() + written, payload.size() - written);
            if (w < 0) { if (errOut) *errOut = "Ошибка записи в сокет"; return false; }
            if (!sock.waitForBytesWritten(5000)) { if (errOut) *errOut = "Таймаут записи"; return false; }
            written += w;
        }
        sock.flush();
        sock.waitForBytesWritten(2000);
        sock.disconnectFromHost();
        sock.waitForDisconnected(2000);
        return true;
    }

private:
    QByteArray m_password;
};
