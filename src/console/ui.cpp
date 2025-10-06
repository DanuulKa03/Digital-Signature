#pragma once

#define NOMINMAX           
#include <windows.h>

#include <iostream>
#include <algorithm>
#include <limits>
#include <string>           


using namespace std;

void SetupConsole() {
    HWND console = GetConsoleWindow(); if (!console) return;
    RECT r; GetWindowRect(console, &r);
    int cw = 900, ch = 520;
    int sw = GetSystemMetrics(SM_CXSCREEN), sh = GetSystemMetrics(SM_CYSCREEN);
    int x = (sw - cw) / 2, y = (sh - ch) / 2;
    MoveWindow(console, x, y, cw, ch, TRUE);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi{}; GetConsoleScreenBufferInfo(hOut, &csbi);
    SHORT ww = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    SHORT wh = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    SetConsoleScreenBufferSize(hOut, { ww,wh });
    LONG style = GetWindowLong(console, GWL_STYLE);
    style &= ~WS_MAXIMIZEBOX; style &= ~WS_SIZEBOX; SetWindowLong(console, GWL_STYLE, style);
}
void SetupConsoleZoom(int fontSize) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_FONT_INFOEX cfi{}; cfi.cbSize = sizeof(cfi);
    GetCurrentConsoleFontEx(hOut, FALSE, &cfi);
    cfi.dwFontSize.Y = fontSize; wcscpy_s(cfi.FaceName, L"Consolas");
    SetCurrentConsoleFontEx(hOut, FALSE, &cfi);
}
static inline string ltrim(string s) { const char* ws = " \t\r\n"; auto p = s.find_first_not_of(ws); if (p == string::npos)return ""; return s.substr(p); }
static inline string rtrim(string s) { const char* ws = " \t\r\n"; auto p = s.find_last_not_of(ws); if (p == string::npos)return ""; return s.substr(0, p + 1); }
string trim(const string& s) { return rtrim(ltrim(s)); }

string readPathLine(const string& prompt) {
    cout << prompt;
    string p; getline(cin, p);
    if (!p.empty() && p.front() == '"' && p.back() == '"') p = p.substr(1, p.size() - 2);
    replace(p.begin(), p.end(), '\\', '/');
    return p;
}
void PrintMenu() {
    system("cls");
    cout << "================================================================================\n";
    cout << "                             СИСТЕМА ЦИФРОВОЙ ПОДПИСИ (NTRUSign)\n";
    cout << "================================================================================\n\n";
    cout << "   [1] Сгенерировать ключи\n";
    cout << "   [2] Подписать файл\n";
    cout << "   [3] Проверить подпись\n";
    cout << "   [0] Выход\n\n";
    cout << "================================================================================\n";
    cout << " Выберите пункт меню: ";
}
void WaitForEnter() { 
    cout << "\nНажмите Enter, чтобы вернуться в меню..."; 
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); }
