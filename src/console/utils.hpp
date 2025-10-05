//
// Created by Daniil Kazakov on 05.10.2025.
//

#pragma once

#include <string>

// ---------------------------- Утилиты путей/ввода ----------------------------
static std::string trim(const std::string& s);

static std::string readPathLine(const std::string& prompt);

// ---- helpers for paths / files ----
static bool ensure_parent_dirs(const std::string& fullPath);

static bool is_directory_like(const std::string& path);

static std::string to_target_file_path(
    const std::string &userPath,
    const std::string& defaultName = "public.key",
    const std::string& defaultExt = ".key");
