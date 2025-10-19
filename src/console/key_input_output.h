#pragma once

#include <string>
#include "math/arithmetic.h"

[[nodiscard]] bool SavePrivateKey(const std::string &userPath);
[[nodiscard]] bool SavePublicKey(const std::string &userPath);
[[nodiscard]] bool LoadPrivateKey(const std::string &path);
[[nodiscard]] bool LoadPublicKey(const std::string &path);
