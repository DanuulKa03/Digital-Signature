#include "util.h"
#include <filesystem>

namespace fs = std::filesystem;

bool ensure_parent_dirs(const std::string& fullPath) {
    try {
        fs::path p(fullPath); fs::path parent = p.parent_path();
        if (!parent.empty() && !fs::exists(parent)) return fs::create_directories(parent);
        return true;
    }
    catch (...) { return false; }
}

bool is_directory_like(const std::string& path) {
    try {
        fs::path p(path);
        if (fs::exists(p)) return fs::is_directory(p);
        if (!path.empty()) { char c = path.back(); return (c == '/' || c == '\\'); }
        return false;
    }
    catch (...) { return false; }
}

std::string to_target_file_path(std::string userPath,
    const std::string& defaultName,
    const std::string& defaultExt)
{
    try {
        fs::path p(userPath);
        if (is_directory_like(userPath)) p /= defaultName;
        else {
            if (!p.has_filename()) p /= defaultName;
            if (!p.has_extension()) p.replace_extension(defaultExt);
        }
        return p.string();
    }
    catch (...) {
        if (is_directory_like(userPath)) return userPath + (userPath.back() == '/' ? "" : "/") + defaultName;
        return userPath + defaultExt;
    }
}
