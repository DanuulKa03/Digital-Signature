#include "util.h"
#include <filesystem>
#include "math/arithmetic.h"

namespace fs = std::filesystem;

bool ensure_parent_dirs(std::string_view fullPath) {
  try {
    const fs::path p(fullPath);
    const fs::path parent = p.parent_path();

    if (!parent.empty() && !fs::exists(parent))
      return fs::create_directories(parent);
    return true;
  } catch (...) {
    return false;
  }
}

bool is_directory_like(std::string_view path) {
  try {
    const fs::path p(path);
    if (fs::exists(p))
      return fs::is_directory(p);
    if (!path.empty()) {
      const char c = path.back();
      return (c == '/' || c == '\\');
    }
    return false;
  } catch (...) {
    return false;
  }
}

std::string to_target_file_path(const std::string &userPath, const std::string &defaultName,
                                std::string_view defaultExt) {
  try {
    fs::path p(userPath);
    if (is_directory_like(userPath))
      p /= defaultName;
    else {
      if (!p.has_filename())
        p /= defaultName;
      if (!p.has_extension())
        p.replace_extension(defaultExt);
    }
    return p.string();
  } catch (...) {
    if (is_directory_like(userPath))
      return userPath + (userPath.back() == '/' ? "" : "/") + defaultName;
    return userPath + defaultExt;
  }
}
