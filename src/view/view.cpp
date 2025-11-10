#include "view.h"
#include <QtCore/QTextStream>
#include <QtCore/QIODevice>
#include <QtCore/QStringConverter>  // <— важно для setEncoding в Qt 6
#include <limits>

static QTextStream qin(stdin,  QIODevice::ReadOnly);
static QTextStream qout(stdout, QIODevice::WriteOnly);

static void ensureUtf8Once() {
    static bool done = false;
    if (done) return;
    qin.setEncoding(QStringConverter::Utf8);
    qout.setEncoding(QStringConverter::Utf8);
    done = true;
}

void ConsoleView::displayMenu() {
    ensureUtf8Once();
    qout << "============================================================\n";
    qout << "         СИСТЕМА ЦИФРОВОЙ ПОДПИСИ (NTRUSign, Qt)\n";
    qout << "============================================================\n\n";
    qout << "  [1] Сгенерировать ключи\n";
    qout << "  [2] Подписать файл\n";
    qout << "  [3] Проверить подпись\n";
    qout << "  [0] Выход\n\n";
    qout << "============================================================\n";
    qout << "Выберите пункт: ";
    qout.flush();
}

QString ConsoleView::prompt(const QString& text) {
    ensureUtf8Once();
    qout << text;
    qout.flush();
    QString line = qin.readLine();
    if (line.size() >= 2 && line.front() == '\"' && line.back() == '\"') {
        line = line.mid(1, line.size() - 2);
    }
    return line.trimmed();
}

void ConsoleView::showMessage(const QString& msg) {
    ensureUtf8Once();
    qout << msg << "\n";
    qout.flush();
}

void ConsoleView::waitForEnter() {
    ensureUtf8Once();
    qout << "\nНажмите Enter, чтобы вернуться в меню...";
    qout.flush();
    (void)qin.readLine();
}
