#pragma once
#include <iostream>
#include <vector>
#include "../MyTypes.h"

namespace Yaz0 
{
	std::vector<u8> Decompress(std::vector<u8> &Data);
	std::vector<u8> Compress(std::vector<u8> &Data, int level = 3, int reserved1 = 0, int reserved2 = 0);
}