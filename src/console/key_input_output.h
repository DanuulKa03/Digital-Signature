#pragma once

#include <string>

bool SavePrivateKey(const std::string& userPath);
bool SavePublicKey(const std::string& userPath);
bool LoadPrivateKey(const std::string& path);
bool LoadPublicKey(const std::string& path);
