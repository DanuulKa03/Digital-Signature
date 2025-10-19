#pragma once
#include <string>

bool ensure_parent_dirs(const std::string& fullPath);
bool is_directory_like(const std::string& path);
std::string to_target_file_path(const std::string &userPath,
    const std::string& defaultName,
    const std::string& defaultExt);
