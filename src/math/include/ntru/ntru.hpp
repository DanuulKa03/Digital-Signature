//
// Created by Daniil Kazakov on 02.10.2025.
//

#pragma once

#include <fstream>

#include "common.hpp"
#include "hash.hpp"

static bool NTRUSign_once(const Poly &m, Poly &s_out);

static bool sign_strict(const std::vector<uint8_t> &msg, Signature &sig);

static bool write_signed(const std::string &inPath, const std::vector<uint8_t> &msg, const Signature &S);

static bool read_signed(const std::string &path, std::vector<uint8_t> &msg, Signature &S, uint64_t &L, int64_t &ts);
