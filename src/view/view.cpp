#include "src/view/view.h"
#include <iostream>

View::View(QObject *parent) : QObject(parent), m_out(stdout), m_in(stdin)
{
}

void View::showMenu()
{
    m_out << "================================================\n";
    m_out << "           NTRUSign Digital Signature\n";
    m_out << "================================================\n\n";
    m_out << "   [1] Generate Keys\n";
    m_out << "   [2] Sign File\n";
    m_out << "   [3] Verify Signature\n";
    m_out << "   [0] Exit\n\n";
    m_out << "================================================\n";
    m_out << " Select option: ";
    m_out.flush();
}

void View::showMessage(const QString& message)
{
    m_out << "[INFO] " << message << "\n";
    m_out.flush();
}

void View::showError(const QString& error)
{
    m_out << "[ERROR] " << error << "\n";
    m_out.flush();
}

QString View::readLine(const QString& prompt)
{
    if (!prompt.isEmpty()) {
        m_out << prompt;
        m_out.flush();
    }

    return m_in.readLine().trimmed();
}

int View::readInt(const QString& prompt)
{
    if (!prompt.isEmpty()) {
        m_out << prompt;
        m_out.flush();
    }

    QString line = m_in.readLine();
    return line.toInt();
}

void View::onOperationCompleted(const QString& message)
{
    showMessage(message);
}

void View::onErrorOccurred(const QString& error)
{
    showError(error);
}