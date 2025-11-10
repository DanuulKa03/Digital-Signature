#include "src/controller/controller.h"
#include "src/model/model.h"
#include "src/view/view.h"
#include <QCoreApplication>

Controller::Controller(Model *model, View *view, QObject *parent)
        : QObject(parent), m_model(model), m_view(view)
{
    if (m_model && m_view) {
        connect(m_model, &Model::operationCompleted, m_view, &View::onOperationCompleted);
        connect(m_model, &Model::errorOccurred, m_view, &View::onErrorOccurred);
    }
}

void Controller::run()
{
    while (true) {
        m_view->showMenu();
        int choice = 0;
        bool ok = false;

        while (!ok) {
            QString input = m_view->readLine();
            choice = input.toInt(&ok);
            if (!ok) {
                m_view->showError("Please enter a number");
                m_view->showMenu();
            }
        }

        if (choice == 0) {
            m_view->showMessage("Exiting program...");
            QCoreApplication::quit();
            break;
        }

        handleMenuChoice(choice);
        waitForEnter();
    }
}

void Controller::handleMenuChoice(int choice)
{
    switch (choice) {
        case 1:
            handleKeyGeneration();
            break;
        case 2:
            handleSignFile();
            break;
        case 3:
            handleVerifySignature();
            break;
        default:
            m_view->showError("Invalid menu option");
            break;
    }
}

void Controller::handleKeyGeneration()
{
    QString paramsFile = getFilePath("Enter parameters file path: ");
    if (paramsFile.isEmpty()) {
        m_view->showError("Parameters file path cannot be empty");
        return;
    }

    if (!m_model->loadParameters(paramsFile)) {
        m_view->showError(m_model->getLastError());
        return;
    }

    if (!m_model->generateKeys()) {
        m_view->showError(m_model->getLastError());
        return;
    }

    QString privKeyPath = getFilePath("Save private key to: ");
    if (privKeyPath.isEmpty() || !m_model->savePrivateKey(privKeyPath)) {
        m_view->showError("Failed to save private key");
        return;
    }

    QString pubKeyPath = getFilePath("Save public key to: ");
    if (pubKeyPath.isEmpty() || !m_model->savePublicKey(pubKeyPath)) {
        m_view->showError("Failed to save public key");
        return;
    }
}

void Controller::handleSignFile()
{
    QString paramsFile = getFilePath("Enter parameters file path: ");
    if (paramsFile.isEmpty() || !m_model->loadParameters(paramsFile)) {
        m_view->showError("Failed to load parameters");
        return;
    }

    QString privKeyPath = getFilePath("Enter private key path: ");
    if (privKeyPath.isEmpty() || !m_model->loadPrivateKey(privKeyPath)) {
        m_view->showError("Failed to load private key");
        return;
    }

    QString filePath = getFilePath("Enter file to sign: ");
    if (filePath.isEmpty()) {
        m_view->showError("File path cannot be empty");
        return;
    }

    Model::Signature signature;
    if (!m_model->signFile(filePath, signature)) {
        m_view->showError("Failed to sign file");
        return;
    }

    QString sigPath = getFilePath("Save signature to: ");
    if (sigPath.isEmpty() || !m_model->saveSignature(sigPath, signature)) {
        m_view->showError("Failed to save signature");
        return;
    }
}

void Controller::handleVerifySignature()
{
    QString filePath = getFilePath("Enter original file path: ");
    if (filePath.isEmpty()) {
        m_view->showError("File path cannot be empty");
        return;
    }

    QString paramsFile = getFilePath("Enter parameters file path: ");
    if (paramsFile.isEmpty() || !m_model->loadParameters(paramsFile)) {
        m_view->showError("Failed to load parameters");
        return;
    }

    QString pubKeyPath = getFilePath("Enter public key path: ");
    if (pubKeyPath.isEmpty() || !m_model->loadPublicKey(pubKeyPath)) {
        m_view->showError("Failed to load public key");
        return;
    }

    QString sigPath = getFilePath("Enter signature file path: ");
    if (sigPath.isEmpty()) {
        m_view->showError("Signature path cannot be empty");
        return;
    }

    Model::Signature signature;
    if (!m_model->loadSignature(sigPath, signature)) {
        m_view->showError("Failed to load signature");
        return;
    }

    if (!m_model->verifySignature(filePath, signature)) {
        m_view->showError("Signature is invalid");
        return;
    }

    m_view->showMessage("Signature is VALID");
}

QString Controller::getFilePath(const QString& prompt)
{
    return m_view->readLine(prompt);
}

void Controller::waitForEnter()
{
    m_view->readLine("\nPress Enter to continue...");
}