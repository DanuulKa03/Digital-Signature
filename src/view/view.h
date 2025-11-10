#ifndef VIEW_H
#define VIEW_H

#include <QObject>
#include <QTextStream>

class View : public QObject
{
Q_OBJECT

public:
    explicit View(QObject *parent = nullptr);
    void showMenu();
    void showMessage(const QString& message);
    void showError(const QString& error);

    QString readLine(const QString& prompt = "");
    int readInt(const QString& prompt = "");

public slots:
    void onOperationCompleted(const QString& message);
    void onErrorOccurred(const QString& error);

private:
    QTextStream m_out;
    QTextStream m_in;
};

#endif // VIEW_H