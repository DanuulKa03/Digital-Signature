//
// Created by Daniil Kazakov on 04.10.2025.
//

#include "arithmetic.hpp"
#include "gauss.hpp"

#include "ntru/keys.hpp"
#include "ntru/ntru.hpp"

#include <iostream>
#include <filesystem>

bool NTRUSign_once(const Poly &m, Poly &s_out) {
  std::vector<int> fI(G_N, 0), gI(G_N, 0), mI(G_N, 0);
  for (int i = 0; i < G_N; ++i) {
    fI[i] = center(G_Fkey[i]);
    gI[i] = center(G_Gkey[i]);
    mI[i] = center(m[i]);
  }

  std::vector<long long> xA(G_N, 0), yA(G_N, 0);
  for (int i = 0; i < G_N; ++i)
    if (mI[i]) {
      for (int j = 0; j < G_N; ++j) {
        int k = i + j;
        if (k >= G_N) k -= G_N;
        xA[k] -= static_cast<long long>(mI[i]) * gI[j];
        yA[k] += static_cast<long long>(mI[i]) * fI[j];
      }
    }

  std::vector<int> kx(G_N, 0), ky(G_N, 0);
  for (int i = 0; i < G_N; ++i) {
    kx[i] = static_cast<int>(llround(static_cast<long double>(xA[i]) / static_cast<long double>(G_Q)));
    ky[i] = static_cast<int>(llround(static_cast<long double>(yA[i]) / static_cast<long double>(G_Q)));
  }

  std::vector<long long> sA(G_N, 0);
  for (int i = 0; i < G_N; ++i) {
    if (kx[i]) for (int j = 0; j < G_N; ++j) {
      int k = i + j;
      if (k >= G_N) k -= G_N;
      sA[k] += static_cast<long long>(kx[i]) * fI[j];
    }
    if (ky[i]) for (int j = 0; j < G_N; ++j) {
      int k = i + j;
      if (k >= G_N) k -= G_N;
      sA[k] += static_cast<long long>(ky[i]) * gI[j];
    }
  }
  s_out.assign(G_N, 0);
  for (int i = 0; i < G_N; ++i) s_out[i] = static_cast<int>(sA[i]);

  Poly sMod(G_N, 0);
  for (int i = 0; i < G_N; ++i) sMod[i] = modQ(s_out[i]);
  const Poly sh = mulModQ(sMod, G_Hpub);
  std::vector<int> tI(G_N, 0);
  for (int i = 0; i < G_N; ++i) tI[i] = center(modQ(static_cast<long long>(sh[i]) - m[i]));

  long double s2 = 0, t2 = 0;
  for (int i = 0; i < G_N; ++i) {
    s2 += static_cast<long double>(s_out[i]) * s_out[i];
    t2 += static_cast<long double>(tI[i]) * tI[i];
  }
  const long double norm2 = s2 + (G_NU * G_NU) * t2;
  return (norm2 <= static_cast<long double>(G_NORM_BOUND) * static_cast<long double>(G_NORM_BOUND));
}

bool sign_strict(const std::vector<uint8_t> &msg, Signature &sig) {
  std::random_device rd;
  std::mt19937 rng(rd());
  for (int tries = 0; tries < G_MAX_SIGN_ATT; ++tries) {
    std::vector<int> y1I(G_N, 0), y2I(G_N, 0);
    for (int i = 0; i < G_N; ++i) {
      y1I[i] = sample_gauss_int(rng, (double) G_SIGMA);
      y2I[i] = sample_gauss_int(rng, (double) G_SIGMA);
    }
    Poly y1(G_N, 0), y2(G_N, 0);
    for (int i = 0; i < G_N; ++i) {
      y1[i] = modQ(y1I[i]);
      y2[i] = modQ(y2I[i]);
    }

    Poly hy1 = mulModQ(G_Hpub, y1);
    Poly z = subMod(y2, hy1);
    auto [e_small, e_mod] = H_e_small(z, msg);

    Poly sI;
    if (!NTRUSign_once(e_mod, sI)) continue;
    Poly sMod(G_N, 0);
    for (int i = 0; i < G_N; ++i) sMod[i] = modQ(sI[i]);
    Poly sh = mulModQ(sMod, G_Hpub);
    std::vector<int> tI(G_N, 0);
    for (int i = 0; i < G_N; ++i) tI[i] = center(modQ(static_cast<long long>(sh[i]) - e_mod[i]));

    Poly x1(G_N, 0), x2(G_N, 0);
    for (int i = 0; i < G_N; ++i) {
      int xi1 = y1I[i] - sI[i];
      int xi2 = y2I[i] - tI[i] - e_small[i];
      x1[i] = modQ(xi1);
      x2[i] = modQ(xi2);
    }

    long double sigma2 = static_cast<long double>(G_SIGMA) * static_cast<long double>(G_SIGMA);
    long double dot = 0.0L, v2 = 0.0L, xnorm2 = 0.0L;
    for (int i = 0; i < G_N; ++i) {
      int xv1 = center(x1[i]), xv2 = center(x2[i]);
      int vv1 = -sI[i], vv2 = -tI[i] - e_small[i];
      dot += static_cast<long double>(xv1) * vv1 + static_cast<long double>(xv2) * vv2;
      v2 += static_cast<long double>(vv1) * vv1 + static_cast<long double>(vv2) * vv2;
      xnorm2 += static_cast<long double>(xv1) * xv1 + static_cast<long double>(xv2) * xv2;
    }
    long double exponent = (dot - 0.5L * v2) / sigma2;
    if (exponent > 700.0L) exponent = 700.0L;
    if (exponent < -700.0L) exponent = -700.0L;
    long double R = expl(exponent);
    long double p = R / G_MACC;
    if (p > 1.0L) p = 1.0L;
    if (!(p == p) || !std::isfinite(static_cast<double>(p))) p = 0.0L;
    std::uniform_real_distribution<double> U(0.0, 1.0);
    if (U(rng) > static_cast<double>(p)) continue;

    long double bound = static_cast<long double>(G_ETA) * static_cast<long double>(G_SIGMA) * sqrtl(2.0L * static_cast<long double>(G_N));
    if (sqrtl(xnorm2) > bound) continue;

    sig.x1 = std::move(x1);
    sig.x2 = std::move(x2);
    sig.e = std::move(e_mod);
    return true;
  }
  return false;
}

bool write_signed(const std::string &inPath, const std::vector<uint8_t> &msg, const Signature &S) {
  std::ofstream out(inPath + ".signed", std::ios::binary);
  if (!out) {
    std::cerr << "Не удалось создать выходные данные\n";
    return false;
  }

  constexpr char magic[4] = {'S', 'G', 'N', '1'};
  out.write(magic, 4);
  auto L = static_cast<uint64_t>(msg.size());
  out.write(reinterpret_cast<const char *>(&L), sizeof(L));

  int64_t ts = 0;
  try {
    const auto ftime = std::filesystem::last_write_time(inPath);
    ts = static_cast<int64_t>(ftime.time_since_epoch().count());
  } catch (...) { ts = 0; }
  out.write(reinterpret_cast<const char *>(&ts), sizeof(ts));

  if (L) out.write(reinterpret_cast<const char *>(msg.data()), (std::streamsize) L);

  auto write_poly_u16 = [&](const Poly &P) {
    for (int i = 0; i < G_N; ++i) {
      auto v = static_cast<uint16_t>(P[i]);
      out.write(reinterpret_cast<const char *>(&v), sizeof(v));
    }
  };
  write_poly_u16(S.x1);
  write_poly_u16(S.x2);
  write_poly_u16(S.e);
  out.close();
  std::cout << "Файл успешно подписан: " << inPath << ".signed\n";
  return true;
}

bool read_signed(const std::string &path, std::pmr::vector<uint8_t> &msg, Signature &S, uint64_t &L, int64_t &ts) {
  std::ifstream in(path, std::ios::binary | std::ios::ate);
  if (!in) {
    std::cerr << "Не удалось открыть файл " << path << "\n";
    return false;
  }
  std::streamoff fileSize = in.tellg();
  in.seekg(0, std::ios::beg);

  char magic[4];
  in.read(magic, 4);
  if (in.gcount() != 4 || magic[0] != 'S' || magic[1] != 'G' || magic[2] != 'N' || magic[3] != '1') {
    std::cout << "Подпись недействительна (bad magic)\n";
    return false;
  }
  in.read(reinterpret_cast<char *>(&L), sizeof(L));
  in.read(reinterpret_cast<char *>(&ts), sizeof(ts));

  size_t expected = 4 + 8 + 8 + static_cast<size_t>(L) + static_cast<size_t>(3 * G_N * 2);
  if (fileSize != static_cast<std::streamoff>(expected)) {
    std::cout << "Подпись недействительна (length mismatch)\n";
    return false;
  }
  msg.resize((size_t) L);
  if (L > 0) in.read(reinterpret_cast<char *>(msg.data()), (std::streamsize) L);

  auto read_poly_u16 = [&](Poly &P) {
    P.assign(G_N, 0);
    for (int i = 0; i < G_N; ++i) {
      uint16_t v;
      in.read(reinterpret_cast<char *>(&v), sizeof(v));
      P[i] = static_cast<int>(v);
    }
  };
  read_poly_u16(S.x1);
  read_poly_u16(S.x2);
  read_poly_u16(S.e);
  return true;
}
