#ifndef NTRU_VIEW_H
#define NTRU_VIEW_H

#include <QString>

/**
 * Console View class for user interaction (input/output in console).
 */
class ConsoleView {
public:
    // Display menu and instructions
  static void displayMenu();
    // Prompt user for a string (with a message) and return the input
    static QString prompt(const QString &message);
    // Display a message to the console (without waiting for input)
    static void showMessage(const QString &message);
    // Display result of signature verification
    static void showVerificationResult(bool isValid);
};

#endif // NTRU_VIEW_H
