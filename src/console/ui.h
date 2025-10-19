#pragma once
#include <string>
#include "math/arithmetic.h"

void SetupConsole();
void SetupConsoleZoom(int fontSize = 22);
void PrintMenu();
void WaitForEnter();
std::string trim(const std::string &s);
std::string readPathLine(const std::string &prompt);
