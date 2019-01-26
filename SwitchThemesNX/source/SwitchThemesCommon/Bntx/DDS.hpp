#pragma once
#include <vector>
#include <string>
#include <tuple>
#include <unordered_map>
#include "../MyTypes.h"
#include "../BinaryReadWrite/Buffer.hpp"
using namespace std;
namespace DDSEncoder 
{
	struct DDSLoadResult 
	{
		s32 width;
		s32 height;
		std::string Format;
		s32 size;
		s32 numMips;
		std::vector<u8> data;
	};

	struct EncoderInfo
	{
		s32 blkHeight;
		s32 blkWidth;
		s32 bpp;
		s32 formatCode;
	};

	struct DDSEncoderResult 
	{
		std::vector<u8> Data;
		EncoderInfo format;
		s32 blockHeightLog2;
	};	

	const std::unordered_map<std::string, EncoderInfo> EncoderTable =
	{
		{ "DXT1",{ 4,4,8 ,0x1a01 } },
		{ "DXT3",{ 4,4,16,0x1b01 } },
		{ "DXT4",{ 4,4,16,0x1c01 } },
		{ "DXT5",{ 4,4,16,0x1c01 } },
	};

	DDSEncoderResult EncodeTex(const DDSLoadResult &img);
	DDSLoadResult LoadDDS(const std::vector<u8> &inb);
}