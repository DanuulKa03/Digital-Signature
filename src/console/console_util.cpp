#include "console_util.h"

#include <algorithm>
#include <locale>
#include <string>
#include <string_view>

#if defined(_WIN32)
  #define NOMINMAX
  #include <windows.h>
#else
  #include <sys/ioctl.h>
  #include <termios.h>
  #include <unistd.h>
#endif

using std::cin;
using std::cout;
using std::string;
using std::string_view;
using std::numeric_limits;

namespace {
#if defined(_WIN32)
bool EnableVTOnWindows() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return false;
    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode)) return false;
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    return SetConsoleMode(hOut, mode) != 0;
}
#endif
} // namespace

void SetupConsole() {
    // Общая часть: ставим локаль по умолчанию (UTF-8 на macOS/Linux),
    // ускоряем iostream.
    try { std::locale::global(std::locale("")); } catch (...) {}
    std::ios::sync_with_stdio(false);
    cin.tie(nullptr);

#if defined(_WIN32)
    // Включим ANSI-последовательности в новых консолях Windows
    EnableVTOnWindows();

    // Можно ограничить размер буфера по аналогии с вашим кодом,
    // но переносить/центровать окно в macOS аналога нет — оставим для Windows:
    HWND console = GetConsoleWindow();
    if (console) {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi{};
        if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
            SHORT ww = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            SHORT wh = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
            SetConsoleScreenBufferSize(hOut, {ww, wh});
        }
        // Уберём maximize/resize, как у вас
        LONG style = GetWindowLong(console, GWL_STYLE);
        style &= ~WS_MAXIMIZEBOX;
        style &= ~WS_SIZEBOX;
        SetWindowLong(console, GWL_STYLE, style);
    }
#else
    // macOS/Linux: тут обычно ничего не делаем.
    // Если нужно узнать размер терминала:
    // struct winsize ws{};
    // if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) { /* ws.ws_col, ws.ws_row */ }
#endif
}

void SetupConsoleZoom(int /*fontSize*/) {
    // На macOS/Linux нельзя программно менять зум/шрифт терминала — NO-OP.
#if defined(_WIN32)
    // Можем оставить вашу реализацию для Windows (необязательно):
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_FONT_INFOEX cfi{};
    cfi.cbSize = sizeof(cfi);
    if (GetCurrentConsoleFontEx(hOut, FALSE, &cfi)) {
        // cfi.dwFontSize.Y = fontSize; // при желании
        wcscpy_s(cfi.FaceName, L"Consolas");
        SetCurrentConsoleFontEx(hOut, FALSE, &cfi);
    }
#endif
}

void ClearScreen() {
    // Кроссплатформенная очистка: ANSI (поддерживается на macOS/Linux и новых Windows).
    // \x1b[2J — очистить, \x1b[H — курсор в левый верхний угол.
    cout << "\x1b[2J\x1b[H";
    cout.flush();
}

static inline string _ltrim_inplace(string s) {
    const char* ws = " \t\r\n";
    auto p = s.find_first_not_of(ws);
    if (p == string::npos) return "";
    return s.substr(p);
}
static inline string _rtrim_inplace(string s) {
    const char* ws = " \t\r\n";
    auto p = s.find_last_not_of(ws);
    if (p == string::npos) return "";
    return s.substr(0, p + 1);
}

string ltrim(string s) { return _ltrim_inplace(std::move(s)); }
string rtrim(string s) { return _rtrim_inplace(std::move(s)); }
string trim(const string& s) { return rtrim(ltrim(s)); }

std::string readPathLine(const std::string& prompt) {
    cout << prompt;
    string p;
    std::getline(cin, p);
    if (!p.empty() && p.front() == '"' && p.back() == '"')
        p = p.substr(1, p.size() - 2);

    // На macOS/Linux пути уже с '/', но нормализация не повредит:
    std::replace(p.begin(), p.end(), '\\', '/');
    return p;
}

void PrintMenu() {
    ClearScreen(); // вместо system("cls")
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
    cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
}
