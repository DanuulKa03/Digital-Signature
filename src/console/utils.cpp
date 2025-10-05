//
// Created by Daniil Kazakov on 05.10.2025.
//

#include <algorithm>
#include <iostream>
#include <filesystem>

#include "console/utils.hpp"

std::string trim(const std::string &s) {
  const auto ws = " \t\r\n";
  const size_t b = s.find_first_not_of(ws);
  if (b == std::string::npos) return "";
  const size_t e = s.find_last_not_of(ws);
  return s.substr(b, e - b + 1);
}

std::string readPathLine(const std::string &prompt) {
  std::cout << prompt;
  std::string p; getline(std::cin, p);
  if (!p.empty() && p.front() == '"' && p.back() == '"')
    p = p.substr(1, p.size() - 2);
  std::ranges::replace(p, '\\', '/');
  return p;
}

bool ensure_parent_dirs(const std::string &fullPath) {
  try {
    const std::filesystem::path p(fullPath);
    const std::filesystem::path parent = p.parent_path();

    if (!parent.empty() && !std::filesystem::exists(parent)) {
      return std::filesystem::create_directories(parent);
    }
    return true;
  }
  catch (...) { return false; }
}

bool is_directory_like(const std::string &path) {
  try {

    const std::filesystem::path p(path);

    if (std::filesystem::exists(p)) return std::filesystem::is_directory(p);
    if (!path.empty()) {
      const char c = path.back();
      return (c == '/' || c == '\\');
    }
    return false;
  }
  catch (...) { return false; }
}

std::string to_target_file_path(const std::string &userPath, const std::string &defaultName, const std::string &defaultExt) {
  try {
    std::filesystem::path p(userPath);
    if (is_directory_like(userPath)) {
      p /= defaultName;
    }
    else {
      if (!p.has_filename()) p /= defaultName;
      if (!p.has_extension()) p.replace_extension(defaultExt);
    }
    return p.string();
  }
  catch (...) {
    if (is_directory_like(userPath))
      return userPath + (userPath.back() == '/' || userPath.back() == '\\' ? "" : "/") + "public.key";
    return userPath + ".key";
  }
}
