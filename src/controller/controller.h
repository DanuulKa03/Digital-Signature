#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QString>

class Model;
class View;

class Controller : public QObject
{
Q_OBJECT

public:
    explicit Controller(Model *model, View *view, QObject *parent = nullptr);
    void run();

private slots:
    void handleMenuChoice(int choice);
    void handleKeyGeneration();
    void handleSignFile();
    void handleVerifySignature();

private:
    Model *m_model;
    View *m_view;

    QString getFilePath(const QString& prompt);
    void waitForEnter();
};

#endif // CONTROLLER_H