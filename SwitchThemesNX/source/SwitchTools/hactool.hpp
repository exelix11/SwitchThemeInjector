#pragma once
#include "../fs.hpp"
#include <string>

bool ExtractPlayerSelectMenu();
bool ExtractUserPage();
bool ExtractHomeMenu();

bool ExtractTitle(u64 titleID, const std::string& Path);

bool ExtractHomeExefs();