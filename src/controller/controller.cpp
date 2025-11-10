#include "src/controller/controller.h"
#include "src/view/view.h"
#include "src/model/model.h"
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

NtruController::NtruController() = default;

void NtruController::run() {
    while (true) {
        ConsoleView::displayMenu();
        QString choiceStr = ConsoleView::prompt(QString());
        bool ok = false;
        int choice = choiceStr.toInt(&ok);
        if (!ok) {
            ConsoleView::showMessage("Неверный ввод. Введите 0..3.");
            ConsoleView::waitForEnter();
            continue;
        }
        if (choice == 0) break;
        if (choice == 1)      { if (!handleKeyGeneration()) ConsoleView::showMessage("[!] Генерация не выполнена."); ConsoleView::waitForEnter(); }
        else if (choice == 2) { if (!handleSigning())       ConsoleView::showMessage("[!] Подпись не выполнена.");   ConsoleView::waitForEnter(); }
        else if (choice == 3) { if (!handleVerification())  ConsoleView::showMessage("[!] Проверка не выполнена.");  ConsoleView::waitForEnter(); }
        else { ConsoleView::showMessage("Неверный пункт."); ConsoleView::waitForEnter(); }
    }
}

bool NtruController::handleKeyGeneration() {
    QString paramsPath = ConsoleView::prompt("Укажите путь к файлу параметров: ");
    NtruModel model;
    if (!model.loadParameters(paramsPath)) return false;
    if (!model.keygen()) return false;

    QString privWhere = ConsoleView::prompt("Куда сохранить ПРИВАТНЫЙ ключ (папка или файл): ");
    if (!model.savePrivateKey(privWhere)) return false;

    QString pubWhere = ConsoleView::prompt("Куда сохранить ОТКРЫТЫЙ ключ (папка или файл): ");
    if (!model.savePublicKey(pubWhere)) return false;

    ConsoleView::showMessage("Генерация завершена успешно.");
    return true;
}

bool NtruController::handleSigning() {
    NtruModel model;

    QString paramsPath = ConsoleView::prompt("Укажите путь к файлу параметров: ");
    if (!model.loadParameters(paramsPath)) return false;

    QString priv = ConsoleView::prompt("Укажите путь к файлу приватного ключа: ");
    if (!model.loadPrivateKey(priv)) return false;

    QString doc = ConsoleView::prompt("Укажите путь к файлу, который нужно подписать: ");
    if (!QFileInfo::exists(doc)) { ConsoleView::showMessage("[!] Файл не найден."); return false; }

    QString sigPath = ConsoleView::prompt("Куда сохранить файл подписи (папка или имя .sig): ");
    if (!model.signFile(doc, sigPath)) return false;

    ConsoleView::showMessage("Подпись создана.");
    return true;
}

bool NtruController::handleVerification() {
    NtruModel model;

    QString doc = ConsoleView::prompt("Укажите путь к исходному файлу (документу): ");
    if (!QFileInfo::exists(doc)) { ConsoleView::showMessage("[!] Файл не найден."); return false; }

    QString paramsPath = ConsoleView::prompt("Укажите путь к файлу параметров: ");
    if (!model.loadParameters(paramsPath)) return false;

    QString pub = ConsoleView::prompt("Укажите путь к файлу открытого ключа: ");
    if (!model.loadPublicKey(pub)) return false;

    QString sigPath = ConsoleView::prompt("Укажите путь к файлу подписи (.sig): ");
    const bool ok = model.verifyFile(doc, sigPath);
    ConsoleView::showMessage(ok ? "Подпись ДЕЙСТВИТЕЛЬНА" : "Подпись недействительна");
    return ok;
}
