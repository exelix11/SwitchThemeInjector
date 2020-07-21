#pragma once
#include <vector>
#include "../MyTypes.h"

namespace Yaz0 
{
	std::vector<u8> Decompress(const std::vector<u8> &Data);
	std::vector<u8> Compress(const std::vector<u8> &Data, int level = 3, int reserved1 = 0, int reserved2 = 0);
}