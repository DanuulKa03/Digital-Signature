#pragma once
#include <QtCore/QString>

class ConsoleView {
public:
    static void displayMenu();
    static QString prompt(const QString& text);
    static void showMessage(const QString& msg);
    static void waitForEnter();
};
