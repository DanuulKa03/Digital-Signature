#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

#include "key_input_output.h"
#include "param_input_output.h"
#include "sign_input_output.h"
#include "ui.h"
#include "util.h"

#include "math/arithmetic.h"
#include "math/hash.h"
#include "math/keys.h"
#include "math/params.h"
#include "math/sign.h"

#define NOMINMAX
#include "console_util.h"

using namespace std;
using namespace ntru;
namespace fs = std::filesystem;

// ---- Флоу ----
static bool KeygenFlow() {

  string params = readPathLine("Укажите путь к файлу параметров: ");
  if (params.empty() || !LoadParametersFile(params)) {
    return false;
  }

  if (!keygen()) {
    cerr << "Не удалось сгенерировать ключи (попробуйте другие параметры)\n";
    return false;
  }

  while (true) {
    string where = readPathLine("Куда сохранить ПРИВАТНЫЙ ключ (папка или файл): ");

    if (where.empty()) {
      cout << "[!] Путь пустой. Повторите.\n";
      continue;
    }

    if (SavePrivateKey(where)) {
      break;
    }
  }
  while (true) {
    string where = readPathLine("Куда сохранить ОТКРЫТЫЙ ключ (папка или файл): ");

    if (where.empty()) {
      cout << "[!] Путь пустой. Повторите.\n";
      continue;
    }

    if (SavePublicKey(where)) {
      break;
    }
  }
  return true;
}
static bool SignFlow() {

  string params = readPathLine("Укажите путь к файлу параметров: ");
  if (params.empty() || !LoadParametersFile(params)) {
    return false;
  }

  string priv = readPathLine("Укажите путь к файлу приватного ключа: ");
  if (priv.empty() || !LoadPrivateKey(priv)) {
    return false;
  }

  string doc = readPathLine("Укажите путь к файлу, который нужно подписать: ");
  if (doc.empty()) {
    cout << "[!] Путь пустой.\n";
    return false;
  }

  ifstream in(doc, ios::binary);
  if (!in) {
    cerr << "Не удалось открыть " << doc << "\n";
    return false;
  }
  vector<uint8_t> msg((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());

  Signature S;

  if (!sign_strict(msg, S)) {
    cerr << "Подпись не удалась (rejection stage)\n";
    return false;
  }

  string sigPath = readPathLine("Куда сохранить файл подписи (папка или имя .sig): ");
  if (sigPath.empty()) {
    cout << "[!] Путь пустой.\n";
    return false;
  }

  if (is_directory_like(sigPath)) {
    fs::path p(sigPath);
    fs::path base = fs::path(doc).filename();
    base += ".sig";
    sigPath = (p / base).string();
  }

  else {
    fs::path p(sigPath);
    if (!p.has_extension())
      p.replace_extension(".sig");
    sigPath = p.string();
  }

  return write_sig(sigPath, S);
}
static bool VerifyFlow() {

  string doc = readPathLine("Укажите путь к исходному файлу (документу): ");
  if (doc.empty() || !fs::exists(doc)) {
    cout << "[!] Файл не найден.\n";
    return false;
  }

  string params = readPathLine("Укажите путь к файлу параметров: ");
  if (params.empty() || !LoadParametersFile(params)) {
    return false;
  }

  string pub = readPathLine("Укажите путь к файлу открытого ключа: ");
  if (pub.empty() || !LoadPublicKey(pub)) {
    return false;
  }

  string sigPath = readPathLine("Укажите путь к файлу подписи (.sig): ");
  if (sigPath.empty()) {
    cout << "[!] Путь пустой.\n";
    return false;
  }

  Signature S;
  if (!read_sig(sigPath, S)) {
    return false;
  }

  ifstream in(doc, ios::binary);
  vector<uint8_t> msg((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());

  Poly hx1 = mulModQ(G_H, S.x1);
  Poly z = subMod(S.x2, hx1);
  EHash eh2 = H_e_small(z, msg);

  for (int i = 0; i < G_N; ++i) {
    if (eh2.e_mod[i] != S.e[i]) {
      cout << "Подпись НЕДЕЙСТВИТЕЛЬНА (hash mismatch)\n";
      return false;
    }
  }
  long double x2norm = 0;
  for (int i = 0; i < G_N; ++i) {
    int a = center(S.x1[i]);
    int b = center(S.x2[i]);
    x2norm += (long double) a * a + (long double) b * b;
  }
  long double bound = (long double) G_ETA * (long double) G_SIGMA * sqrtl(2.0L * (long double) G_N);

  if (sqrtl(x2norm) > bound) {
    cout << "Подпись НЕДЕЙСТВИТЕЛЬНА (norm)\n";
    return false;
  }

  cout << "Подпись ДЕЙСТВИТЕЛЬНА\n";
  return true;
}

int main() {
  setlocale(LC_ALL, "Rus");
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  SetupConsoleZoom(22);
  SetupConsole();

  while (true) {
    PrintMenu();
    int c;

    if (!(cin >> c)) {
      return 0;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    switch (c) {
      case 0:
        break;

      case 1:
        if (!KeygenFlow())
          cout << "[!] Генерация не выполнена.\n";
        WaitForEnter();
        break;

      case 2:
        if (!SignFlow())
          cout << "[!] Подпись не выполнена.\n";
        WaitForEnter();
        break;

      case 3:
        if (!VerifyFlow())
          cout << "[!] Проверка не выполнена.\n";
        WaitForEnter();
        break;

      default:
        cout << "Неверный пункт.\n";
        WaitForEnter();
        break;
    }

    return 0;
  }
