#include "src/controller/controller.h"
#include <QFile>

NtruController::NtruController() : paramsLoaded(false) {
    // Initialize flag
    paramsLoaded = false;
}

void NtruController::run() {
    while (true) {
        ConsoleView::displayMenu();
        QString choiceStr = ConsoleView::prompt("Enter your choice: ");
        if (choiceStr.isEmpty()) {
            // If input was closed or empty, exit
            break;
        }
        bool ok = false;
        int choice = choiceStr.toInt(&ok);
        if (!ok) {
            ConsoleView::prompt("Invalid input. Please enter a number from the menu.");
            continue;
        }
        switch (choice) {
            case 1:
                handleKeyGeneration();
                break;
            case 2:
                handleSigning();
                break;
            case 3:
                handleVerification();
                break;
            case 0:
                ConsoleView::showMessage("Exiting application.");
                return;
            default:
                ConsoleView::showMessage("Invalid choice. Please select 1, 2, 3, or 0.");
        }
    }
}

void NtruController::handleKeyGeneration() {
    QString paramPath = ConsoleView::prompt("Enter path to parameter file: ");
    if (paramPath.isEmpty()) {
        return;
    }
    NtruParams newParams{};
    if (!NtruModel::loadParameters(paramPath, newParams)) {
        ConsoleView::showMessage("Failed to load parameters from file.");
        return;
    }
    // Generate keys
    NtruPrivKey priv;
    NtruPubKey pub;
    if (!NtruModel::generateKeyPair(newParams, priv, pub)) {
        ConsoleView::showMessage("Key generation failed (could not find invertible polynomial).");
        return;
    }
    // Ask for output file paths
    QString privPath = ConsoleView::prompt("Enter output file path for Private Key: ");
    if (privPath.isEmpty()) {
        return;
    }
    const QString pubPath = ConsoleView::prompt("Enter output file path for Public Key: ");
    if (pubPath.isEmpty()) {
        return;
    }
    if (!NtruModel::savePrivateKey(privPath, newParams, priv)) {
        ConsoleView::showMessage("Error saving private key to file.");
        return;
    }
    if (!NtruModel::savePublicKey(pubPath, newParams, pub)) {
        ConsoleView::showMessage("Error saving public key to file.");
        return;
    }
    // Store loaded params for subsequent operations (if needed)
    params = newParams;
    paramsLoaded = true;
    ConsoleView::showMessage("Key pair generated and saved successfully.");
}

void NtruController::handleSigning() {
  const QString paramPath = ConsoleView::prompt("Enter path to parameter file: ");
    if (paramPath.isEmpty()) {
        return;
    }
    NtruParams signParams{};
    if (!NtruModel::loadParameters(paramPath, signParams)) {
        ConsoleView::showMessage("Failed to load parameters. Ensure file contains N, q, d, ν.");
        return;
    }
    const QString privPath = ConsoleView::prompt("Enter path to Private Key file: ");
    if (privPath.isEmpty()) {
        return;
    }
    NtruPrivKey priv;
    if (!NtruModel::loadPrivateKey(privPath, signParams, priv)) {
        ConsoleView::showMessage("Failed to load private key from file.");
        return;
    }
    const QString msgPath = ConsoleView::prompt("Enter path to the message file to sign: ");
    if (msgPath.isEmpty()) {
        return;
    }
    // Check that message file exists
    if (!QFile::exists(msgPath)) {
        ConsoleView::showMessage("Message file not found.");
        return;
    }
    const QString sigPath = ConsoleView::prompt("Enter output file path for the signature: ");
    if (sigPath.isEmpty()) {
        return;
    }
    if (NtruModel::signMessage(signParams, priv, msgPath, sigPath)) {
        ConsoleView::showMessage("File signed successfully. Signature saved to: " + sigPath);
    } else {
        ConsoleView::showMessage("Signing failed.");
    }
}

void NtruController::handleVerification() {
  const QString paramPath = ConsoleView::prompt("Enter path to parameter file: ");
    if (paramPath.isEmpty()) {
        return;
    }
    NtruParams verParams{};
    if (!NtruModel::loadParameters(paramPath, verParams)) {
        ConsoleView::showMessage("Failed to load parameters file.");
        return;
    }
    const QString pubPath = ConsoleView::prompt("Enter path to Public Key file: ");
    if (pubPath.isEmpty()) {
        return;
    }
    NtruPubKey pub;
    if (!NtruModel::loadPublicKey(pubPath, verParams, pub)) {
        ConsoleView::showMessage("Failed to load public key from file.");
        return;
    }
    const QString msgPath = ConsoleView::prompt("Enter path to the original message file: ");
    if (msgPath.isEmpty()) {
        return;
    }
    if (!QFile::exists(msgPath)) {
        ConsoleView::showMessage("Message file not found.");
        return;
    }
    const QString sigPath = ConsoleView::prompt("Enter path to the signature file: ");
    if (sigPath.isEmpty()) {
        return;
    }
    bool valid = false;
    if (NtruModel::verifyMessage(verParams, pub, msgPath, sigPath)) {
        valid = true;
    }
    ConsoleView::showVerificationResult(valid);
}
