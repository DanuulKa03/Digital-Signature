#include <fstream>
#include <iostream>

#include "common.hpp"
#include "arithmetic.hpp"
#include "hash.hpp"
#include "console/utils.hpp"
#include "ntru/keys.hpp"
#include "ntru/ntru.hpp"

// ---------------------------- Загрузка/сохранение параметров и ключей ----------------------------
static bool LoadParameters(const std::string &paramPath) {
  std::ifstream in(paramPath);
  if (!in) {
    std::cerr << "Не удалось открыть файл параметров: " << paramPath << "\n";
    return false;
  }

  std::string line;
  int have = 0;
  while (getline(in, line)) {
    line = trim(line);
    if (line.empty() || line[0] == '#') continue;
    const size_t eq = line.find('=');
    if (eq == std::string::npos) continue;
    std::string k = trim(line.substr(0, eq)), v = trim(line.substr(eq + 1));
    if (k == "N") {
      G_N = stoi(v);
      have++;
    } else if (k == "Q") {
      G_Q = stoi(v);
      have++;
    } else if (k == "D") {
      G_D = stoi(v);
      have++;
    } else if (k == "NU") {
      G_NU = stod(v);
      have++;
    } else if (k == "NORM_BOUND") {
      G_NORM_BOUND = stoi(v);
      have++;
    } else if (k == "ETA") {
      G_ETA = stod(v);
      have++;
    } else if (k == "ALPHA") {
      G_ALPHA = stoi(v);
      have++;
    } else if (k == "SIGMA") {
      G_SIGMA = stoi(v);
      have++;
    } else if (k == "MAX_SIGN_ATTEMPTS_MASK") { G_MAX_SIGN_ATT = stoi(v); }
  }
  if (have < 8) {
    std::cerr << "Файл параметров неполный. Требуются: N,Q,D,NU,NORM_BOUND,ETA,ALPHA,SIGMA\n";
    return false;
  }
  if (G_N <= 0 || G_Q <= 0 || G_D <= 0 || G_SIGMA <= 0 || G_ALPHA < 0) {
    std::cerr << "Некорректные значения параметров.\n";
    return false;
  }

  G_MACC = std::exp(1.0 + 1.0 / (2.0 * static_cast<double>(G_ALPHA) * static_cast<double>(G_ALPHA)));
  G_Fkey.assign(G_N, 0);
  G_Gkey.assign(G_N, 0);
  G_Hpub.assign(G_N, 0);
  return true;
}

static bool SavePublicKeyAtLocation(const std::string &userPath) {
  const std::string finalPath = to_target_file_path(userPath);
  if (!ensure_parent_dirs(finalPath)) {
    std::cout << "Не удалось создать родительские каталоги для: " << finalPath << "\n";
    return false;
  }
  std::ofstream out(finalPath, std::ios::binary | std::ios::trunc);
  if (!out) {
    std::cout << "Не удалось создать файл открытого ключа: " << finalPath << "\n";
    return false;
  }
  out << G_N << "\n";
  for (int i = 0; i < G_N; ++i) {
    out << G_Hpub[i];
    if (i + 1 < G_N) out << ' ';
  }
  out << "\n";
  out.close();
  std::cout << "Открытый ключ сохранён в: " << finalPath << "\n";
  return true;
}

static bool LoadPublicKey(const std::string &pubPath) {
  std::ifstream in(pubPath);
  if (!in) {
    std::cerr << "Не удалось открыть файл открытого ключа: " << pubPath << "\n";
    return false;
  }
  int n_in = 0;
  if (!(in >> n_in)) {
    std::cerr << "Некорректный формат открытого ключа (ожидался N)\n";
    return false;
  }
  if (n_in != G_N) {
    std::cerr << "Несоответствие N: params.N=" << G_N << ", key.N=" << n_in << "\n";
    return false;
  }
  G_Hpub.assign(G_N, 0);
  for (int i = 0; i < G_N; ++i) {
    long long v;
    if (!(in >> v)) {
      std::cerr << "Недостаточно коэффициентов в файле ключа\n";
      return false;
    }
    G_Hpub[i] = modQ(v);
  }
  return true;
}

// ---------------------------- Верхний уровень ----------------------------
static bool SignFile(const std::string &path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    std::cerr << "Не удалось открыть файл: " << path << "\n";
    return false;
  }
  std::vector<uint8_t> msg((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

  Signature S;
  if (!sign_strict(msg, S)) {
    std::cerr << "Подпись не удалась (rejection stage)\n";
    return false;
  }
  return write_signed(path, msg, S);
}

static bool VerifyFileExternal(const std::string &signedPath, const std::string &origPath) {
  std::vector <uint8_t> msg;
  Signature S;
  uint64_t L = 0;
  int64_t ts = 0;
  if (!read_signed(signedPath, msg, S, L, ts)) return false;

  if (!std::filesystem::exists(origPath)) {
    std::cout << "Исходный файл отсутствует → подпись недействительна\n";
    return false;
  }
  int64_t ts_now = 0;
  try {
    auto ftime = std::filesystem::last_write_time(origPath);
    ts_now = (int64_t) ftime.time_since_epoch().count();
  } catch (...) { ts_now = 0; }
  if (ts_now != ts) {
    std::cout << "Подпись недействительна (файл был модифицирован)\n";
    return false;
  }

  Poly hx1 = mulModQ(G_Hpub, S.x1);
  Poly z = subMod(S.x2, hx1);
  EHash eh2 = H_e_small(z, msg);
  for (int i = 0; i < G_N; ++i) {
    if (eh2.e_mod[i] != S.e[i]) {
      std::cout << "Подпись недействительна (hash mismatch)\n";
      return false;
    }
  }

  long double x2norm = 0;
  for (int i = 0; i < G_N; ++i) {
    const int a = center(S.x1[i]);
    const int b = center(S.x2[i]);
    x2norm += static_cast<long double>(a) * a + static_cast<long double>(b) * b;
  }

  const long double bound = static_cast<long double>(G_ETA) * static_cast<long double>(G_SIGMA) * sqrtl(2.0L * static_cast<long double>(G_N));

  if (sqrtl(x2norm) > bound) {
    std::cout << "Подпись недействительна (norm)\n";
    return false;
  }

  std::cout << "Подпись ДЕЙСТВИТЕЛЬНА для файла: " << origPath << "\n";
  return true;
}

static bool ExtractMessage(const std::string &signedPath) {
  std::ifstream in(signedPath, std::ios::binary | std::ios::ate);
  if (!in) {
    std::cerr << "Не удалось открыть " << signedPath << "\n";
    return false;
  }
  std::streamoff fileSize = in.tellg();
  in.seekg(0, std::ios::beg);

  char magic[4];
  in.read(magic, 4);
  if (in.gcount() != 4 || magic[0] != 'S' || magic[1] != 'G' || magic[2] != 'N' || magic[3] != '1') {
    std::cout << "Не подписанный файл\n";
    return false;
  }

  uint64_t L = 0;
  int64_t ts = 0;
  in.read(reinterpret_cast<char *>(&L), sizeof(L));
  in.read(reinterpret_cast<char *>(&ts), sizeof(ts));
  const size_t expected = 4 + 8 + 8 + static_cast<size_t>(L) + static_cast<size_t>(3 * G_N * 2);
  if (fileSize < static_cast<std::streamoff>(expected)) {
    std::cout << "Подписанный файл поврежден\n";
    return false;
  }

  std::vector<uint8_t> msg(L);
  if (L > 0) in.read(reinterpret_cast<char *>(msg.data()), (std::streamsize) L);
  std::string outPath = signedPath + ".restored.txt";
  std::ofstream out(outPath, std::ios::binary);
  if (!out) {
    std::cerr << "Не удалось создать " << outPath << "\n";
    return false;
  }
  out.write(reinterpret_cast<const char *>(msg.data()), (std::streamsize) L);
  out.close();
  std::cout << "Исходное сообщение помещено в: " << outPath << "\n";
  return true;
}

// ---------------------------- Меню и main ----------------------------
static void PrintMenu() {
  system("cls");
  std::cout << "================================================================================\n";
  std::cout << "                             СИСТЕМА ЦИФРОВОЙ ПОДПИСИ\n";
  std::cout << "================================================================================\n\n";
  std::cout << "   [1] Подписать файл\n";
  std::cout << "   [2] Проверить подпись\n";
  std::cout << "   [3] Восстановить исходный файл из .signed\n";
  std::cout << "   [0] Выход\n\n";
  std::cout << "================================================================================\n";
  std::cout << " Выберите пункт меню: ";
}

static void WaitForEnter() {
  std::cout << "\nНажмите Enter, чтобы вернуться в меню...";
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  setlocale(LC_ALL, "Rus");

  // WinAPI «украшалки»
  SetupConsoleZoom(22);
  SetupConsole();
  EnableScrollbars();

  while (true) {
    PrintMenu();
    int c;
    if (!(std::cin >> c)) return 0;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (c == 0) break;

    else if (c == 1) {
      // Подписать
      std::cout << "\n";
      std::string paramPath = readPathLine("Укажите путь к файлу параметров: ");
      if (paramPath.empty() || !LoadParameters(paramPath)) {
        WaitForEnter();
        continue;
      }

      if (!keygen()) {
        std::cerr << "Не удалось сгенерировать ключи (F невырожден по mod 2?)\n";
        WaitForEnter();
        continue;
      }

      while (true) {
        std::string where = readPathLine("Укажите путь к МЕСТУ сохранения открытого ключа (папка или файл): ");
        if (where.empty()) {
          std::cout << "[!] Путь пустой. Повторите.\n";
          continue;
        }
        if (SavePublicKeyAtLocation(where)) break; // дальше только при успехе записи
      }

      std::string filePath = readPathLine("Укажите путь к файлу, который нужно подписать: ");
      if (filePath.empty()) {
        std::cout << "[!] Путь пустой. Повторите.\n";
        WaitForEnter();
        continue;
      }

      if (!SignFile(filePath)) { std::cerr << "Подпись не удалась.\n"; }
      WaitForEnter();
    } else if (c == 2) {
      // Проверить
      std::cout << "\n";
      std::string paramPath = readPathLine("Укажите путь к файлу параметров: ");
      if (paramPath.empty() || !LoadParameters(paramPath)) {
        WaitForEnter();
        continue;
      }

      std::string pubPath = readPathLine("Укажите путь к файлу открытого ключа: ");
      if (pubPath.empty() || !LoadPublicKey(pubPath)) {
        WaitForEnter();
        continue;
      }

      std::string signedPath = readPathLine("Укажите путь к подписанному файлу (*.signed): ");
      if (signedPath.empty()) {
        std::cout << "[!] Путь пустой. Повторите.\n";
        WaitForEnter();
        continue;
      }

      std::string origPath = signedPath;
      size_t pos = origPath.rfind(".signed");
      if (pos != std::string::npos) origPath.erase(pos);

      if (!VerifyFileExternal(signedPath, origPath)) { std::cerr << "Проверка не пройдена.\n"; }
      WaitForEnter();
    } else if (c == 3) {
      // Восстановить исходный из .signed
      std::cout << "\n";
      std::string paramPath = readPathLine("Укажите путь к файлу параметров (для знания N): ");
      if (paramPath.empty() || !LoadParameters(paramPath)) {
        WaitForEnter();
        continue;
      }

      std::string p = readPathLine("Укажите путь к .signed: ");
      if (p.empty()) {
        std::cout << "[!] Путь пустой. Повторите.\n";
        WaitForEnter();
        continue;
      }
      ExtractMessage(p);
      WaitForEnter();
    } else {
      std::cout << "Неверный пункт.\n";
    }
  }
  return 0;
}
