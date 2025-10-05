//
// Created by Daniil Kazakov on 05.10.2025.
//

#include "console/settings.hpp"

#include <windows.h>

void SetupConsole() {
  HWND console = GetConsoleWindow();
  if (!console) return;

  // Центрируем и задаём размер окна
  RECT r; GetWindowRect(console, &r);
  int consoleWidth = 900;
  int consoleHeight = 520;

  int screenWidth = GetSystemMetrics(SM_CXSCREEN);
  int screenHeight = GetSystemMetrics(SM_CYSCREEN);
  int posX = (screenWidth - consoleWidth) / 2;
  int posY = (screenHeight - consoleHeight) / 2;
  MoveWindow(console, posX, posY, consoleWidth, consoleHeight, TRUE);

  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

  // Подгоняем буфер под окно (уберём полосу прокрутки)
  CONSOLE_SCREEN_BUFFER_INFO csbi{};
  GetConsoleScreenBufferInfo(hOut, &csbi);
  SHORT winWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
  SHORT winHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
  COORD bufferSize = { winWidth, winHeight };
  SetConsoleScreenBufferSize(hOut, bufferSize);

  // Запрет растягивания окна
  LONG style = GetWindowLong(console, GWL_STYLE);
  style &= ~WS_MAXIMIZEBOX;
  style &= ~WS_SIZEBOX;
  SetWindowLong(console, GWL_STYLE, style);
}

void EnableScrollbars() {
  // Если нужно — включаем прокрутку (удвоим высоту буфера)
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO csbi{};
  GetConsoleScreenBufferInfo(hOut, &csbi);
  COORD newSize = { csbi.dwSize.X, (SHORT)(max<int>(csbi.dwSize.Y, csbi.srWindow.Bottom - csbi.srWindow.Top + 1) * 2) };
  SetConsoleScreenBufferSize(hOut, newSize);
}

void SetupConsoleZoom(int fontSize) {
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_FONT_INFOEX cfi{}; cfi.cbSize = sizeof(cfi);
  GetCurrentConsoleFontEx(hOut, FALSE, &cfi);
  cfi.dwFontSize.Y = fontSize;
  cfi.dwFontSize.X = 0;
  wcscpy_s(cfi.FaceName, L"Consolas");
  SetCurrentConsoleFontEx(hOut, FALSE, &cfi);
}
