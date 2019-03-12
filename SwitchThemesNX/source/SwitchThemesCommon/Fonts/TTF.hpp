#pragma once
#include "../BinaryReadWrite/Buffer.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include "../MyTypes.h"

namespace SwitchThemesCommon::TTF
{
	std::string GetFontName(const std::vector<u8> &Data);
	std::vector<u8> ConvertToBFTTF(const std::vector<u8> &Data);
}