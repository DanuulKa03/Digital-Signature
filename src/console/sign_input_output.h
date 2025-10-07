#pragma once
#include <string>
#include "math/types.h"

bool write_sig(const std::string& sigPath, const ntru::Signature& S);
bool read_sig(const std::string& sigPath, ntru::Signature& S);
