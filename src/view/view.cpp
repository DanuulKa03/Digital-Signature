#include "src/view/view.h"
#include <QTextStream>
#include <iostream>

void ConsoleView::displayMenu() {
    QTextStream qout(stdout);
    qout << "================ NTRUSign Console Application ================\n";
    qout << "Select an option:\n";
    qout << "1. Generate keys (Key Generation)\n";
    qout << "2. Sign a file (Generate Signature)\n";
    qout << "3. Verify a signature\n";
    qout << "0. Exit\n";
    qout.flush();
}

QString ConsoleView::prompt(const QString &message) {
    QTextStream qout(stdout);
    QTextStream qin(stdin);
    qout << message;
    qout.flush();
    const QString input = qin.readLine();
    if (input.isNull()) {
        return {};
    }
    return input.trimmed();
}

void ConsoleView::showMessage(const QString &message) {
    QTextStream qout(stdout);
    qout << message << "\n";
    qout.flush();
}

void ConsoleView::showVerificationResult(bool isValid) {
    QTextStream qout(stdout);
    if (isValid) {
        qout << "ДЕЙСТВИТЕЛЬНА\n";    // Signature is valid
    } else {
        qout << "НЕДЕЙСТВИТЕЛЬНА\n";  // Signature is invalid
    }
    qout.flush();
}
