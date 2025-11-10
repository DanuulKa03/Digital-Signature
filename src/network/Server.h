
#pragma once
#include <QtCore>
#include <QtNetwork>
#include "FileReceiver.h"

class Server : public QObject {
    Q_OBJECT
public:
    explicit Server(QObject* parent=nullptr) : QObject(parent), m_server(new QTcpServer(this)) {}

    bool start(const QHostAddress& addr, quint16 port, const QString& password, const QString& outDir="/data/out") {
        m_receiver.reset(new FileReceiver(password.toUtf8(), this));
        m_receiver->setOutputDir(outDir);
        connect(m_server, &QTcpServer::newConnection, this, &Server::onNewConnection);
        return m_server->listen(addr, port);
    }

signals:
    void fileSaved(const QString& path);
    void error(const QString& message);

private slots:
    void onNewConnection() {
        while (m_server->hasPendingConnections()) {
            QScopedPointer<QTcpSocket> sock(m_server->nextPendingConnection());
            QString saved, err;
            if (!m_receiver->handle(sock.data(), &saved, &err)) {
                emit error(err);
            } else {
                emit fileSaved(saved);
            }
            sock->disconnectFromHost();
        }
    }

private:
    QTcpServer* m_server;
    QScopedPointer<FileReceiver> m_receiver;
};
