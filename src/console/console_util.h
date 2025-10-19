#pragma once
#include <string>
#include <limits>
#include <iostream>

void SetupConsole();                  // кроссплатформенная подготовка терминала (UTF-8, ANSI)
void SetupConsoleZoom(int fontSize);  // NO-OP на macOS/Linux
void ClearScreen();                   // очистка экрана (ANSI)
void PrintMenu();                     // вывод меню
void WaitForEnter();                  // пауза до Enter

std::string ltrim(std::string s);
std::string rtrim(std::string s);
std::string trim(const std::string& s);
std::string readPathLine(const std::string& prompt);
