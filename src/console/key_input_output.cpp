#include <filesystem>
#include <fstream>
#include <iostream>

#include "math/arithmetic.h"
#include "math/keys.h"
#include "math/params.h"

#include "common.hpp"
#include "key_input_output.h"
#include "util.h"

namespace fs = std::filesystem;

bool SavePrivateKey(std::string_view userPath) {
  const std::string path = to_target_file_path(userPath, "private.key", ".key");

  if (!ensure_parent_dirs(path)) {
    std::cout << "�� ������� ������� �������� ���: " << path << std::endl;
    return false;
  }

  std::ofstream out(path, std::ios::binary | std::ios::trunc);

  if (!out) {
    std::cout << "�� ������� ������� ���� ���������� �����: " << path << std::endl;
    return false;
  }

  out << "PRIV1" << std::endl << G_N << std::endl;
  for (int i = 0; i < G_N; ++i) {
    out << G_F[i];
    if (i + 1 < G_N) {
      out << ' ';
    }
  }
  out << std::endl;
  for (int i = 0; i < G_N; ++i) {
    out << G_G[i];
    if (i + 1 < G_N) {
      out << ' ';
    }
  }
  out << std::endl;
  out.close();
  std::cout << "��������� ���� �������: " << path << std::endl;
  return true;
}
bool SavePublicKey(std::string_view userPath) {

  std::string path = to_target_file_path(userPath, "public.key", ".key");
  if (!ensure_parent_dirs(path)) {
    std::cout << "�� ������� ������� �������� ���: " << path << std::endl;
    return false;
  }
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  if (!out) {
    std::cout << "�� ������� ������� ���� ��������� �����: " << path << std::endl;
    return false;
  }

  out << "PUB1" << std::endl << G_N << std::endl;
  for (int i = 0; i < G_N; ++i) {
    out << G_H[i];
    if (i + 1 < G_N) {
      out << ' ';
    }
  }

  out << std::endl;
  out.close();
  std::cout << "�������� ���� �������: " << path << std::endl;
  return true;
}
bool LoadPrivateKey(std::string_view path) {

  std::ifstream in(path);
  if (!in) {
    std::cerr << "�� ������� ������� ��������� ����: " << path << std::endl;
    return false;
  }

  std::string hdr;
  if (!(in >> hdr) || hdr != "PRIV1") {
    std::cerr << "�������� ������ ���������� �����" << std::endl;
    return false;
  }

  if (int n; !(in >> n) || n != G_N) {
    std::cerr << "N �� ��������� � �����������" << std::endl;
    return false;
  }

  G_F.assign(G_N, 0);
  G_G.assign(G_N, 0);

  for (int i = 0; i < G_N; ++i) {
    long long v;
    if (!(in >> v)) {
      std::cerr << "������������ ������������� f\n";
      return false;
    }
    G_F[i] = modQ(v);
  }

  for (int i = 0; i < G_N; ++i) {
    long long v;
    if (!(in >> v)) {
      std::cerr << "������������ ������������� g" << std::endl;
      return false;
    }
    G_G[i] = ntru::modQ(v);
  }

  Poly inv2(G_N, 0);
  if (!invertMod2(G_F, inv2)) {
    std::cerr << "f �� ������������� �� mod 2" << std::endl;
    return false;
  }

  Poly Finv = henselLiftToQ(G_F, inv2);
  G_H = mulModQ(Finv, G_G);
  return true;
}
bool LoadPublicKey(std::string_view path) {
  std::ifstream in(path);

  if (!in) {
    std::cerr << "�� ������� ������� ��������� ����: " << path << std::endl;
    return false;
  }

  std::string hdr;
  if (!(in >> hdr) || hdr != "PUB1") {
    std::cerr << "�������� ������ ���������� �����" << std::endl;
    return false;
  }

  int n;
  if (!(in >> n) || n != G_N) {
    std::cerr << "N �� ��������� � �����������\n";
    return false;
  }

  G_H.assign(G_N, 0);
  for (int i = 0; i < G_N; ++i) {
    long long v;
    if (!(in >> v)) {
      std::cerr << "������������ ������������� h\n";
      return false;
    }
    G_H[i] = modQ(v);
  }
  return true;
}
