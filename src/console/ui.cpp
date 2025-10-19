#pragma once

#define NOMINMAX           
#include <algorithm>
#include <windows.h>

#include <iostream>

void SetupConsole() {
  HWND console = GetConsoleWindow();
  if (!console) return;
  RECT r;
  GetWindowRect(console, &r);
  int cw = 900, ch = 520;
  int sw = GetSystemMetrics(SM_CXSCREEN), sh = GetSystemMetrics(SM_CYSCREEN);
  int x = (sw - cw) / 2, y = (sh - ch) / 2;
  MoveWindow(console, x, y, cw, ch, TRUE);
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO csbi{};
  GetConsoleScreenBufferInfo(hOut, &csbi);
  SHORT ww = csbi.srWindow.Right - csbi.srWindow.Left + 1;
  SHORT wh = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
  SetConsoleScreenBufferSize(hOut, {ww, wh});
  LONG style = GetWindowLong(console, GWL_STYLE);
  style &= ~WS_MAXIMIZEBOX;
  style &= ~WS_SIZEBOX;
  SetWindowLong(console, GWL_STYLE, style);
}

void SetupConsoleZoom(int fontSize) {
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_FONT_INFOEX cfi{};
  cfi.cbSize = sizeof(cfi);
  GetCurrentConsoleFontEx(hOut, FALSE, &cfi);
  cfi.dwFontSize.Y = fontSize;
  wcscpy_s(cfi.FaceName, L"Consolas");
  SetCurrentConsoleFontEx(hOut, FALSE, &cfi);
}

static inline std::string ltrim(std::string s) {
  const char *ws = " \t\r\n";
  auto p = s.find_first_not_of(ws);
  if (p == std::string::npos)return "";
  return s.substr(p);
}

static inline std::string rtrim(std::string s) {
  const char *ws = " \t\r\n";
  auto p = s.find_last_not_of(ws);
  if (p == std::string::npos)return "";
  return s.substr(0, p + 1);
}

std::string trim(const std::string &s) { return rtrim(ltrim(s)); }

std::string readPathLine(const std::string &prompt) {
  std::cout << prompt;
  std::string p;
  getline(std::cin, p);
  if (!p.empty() && p.front() == '"' && p.back() == '"') p = p.substr(1, p.size() - 2);
  std::ranges::replace(p, '\\', '/');
  return p;
}

void PrintMenu() {
  system("cls");
  std::cout << "================================================================================\n";
  std::cout << "                             СИСТЕМА ЦИФРОВОЙ ПОДПИСИ (NTRUSign)\n";
  std::cout << "================================================================================\n\n";
  std::cout << "   [1] Сгенерировать ключи\n";
  std::cout << "   [2] Подписать файл\n";
  std::cout << "   [3] Проверить подпись\n";
  std::cout << "   [0] Выход\n\n";
  std::cout << "================================================================================\n";
  std::cout << " Выберите пункт меню: ";
}

void WaitForEnter() {
  std::cout << std::endl << "Нажмите Enter, чтобы вернуться в меню...";
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
