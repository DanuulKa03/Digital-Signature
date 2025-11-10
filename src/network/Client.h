
#pragma once
#include <QtCore>

#include "FileSender.h"

class Client {
public:
    explicit Client(const QString& password) : m_password(password.toUtf8()) {}
    bool sendEncryptedFile(const QString& host, quint16 port, const QString& localPath, const QString& remoteName, QString* errOut=nullptr) {
        FileSender s(m_password);
        return s.send(host, port, localPath, remoteName, errOut);
    }
private:
    QByteArray m_password;
};
