#pragma once
#include <vector>
#include <string>
#include <tuple>
#include "../MyTypes.h"
#include "../BinaryReadWrite/Buffer.hpp"

using namespace std;

namespace DDSEncoder 
{
	struct DDSLoadResult 
	{
		s32 width;
		s32 height;
		s32 format_;
		string fourcc;
		s32 size;
		vector<s32> compSel;
		s32 numMips;
		vector<u8> data;
	};

	std::vector<u8> EncodeTex(const DDSLoadResult &img);
	DDSLoadResult LoadDDS(const std::vector<u8> &inb);
}