#include <QtCore/QCoreApplication>
#include "src/controller/controller.h"

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

int main(int argc, char* argv[]) {

#ifdef Q_OS_WIN
    // Переводим ввод/вывод консоли Windows в UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    // (Опционально) включить ANSI-escape/VT-режим, если нужно:
    // HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    // DWORD dwMode = 0; GetConsoleMode(hOut, &dwMode);
    // SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif

    QCoreApplication app(argc, argv);
    NtruController controller;
    controller.run();               // бесконечный цикл меню внутри
    return 0;
}
