#pragma once
#include "../BinaryReadWrite/Buffer.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include "../MyTypes.h"

class SARC
{
public:
	struct PackedSarc 
	{
		std::vector<u8> data;
		u32 align;
	};

	struct SarcData
	{
		std::unordered_map<std::string, std::vector<u8>> files;
		Endianness endianness;
		bool HashOnly;
	};

	static PackedSarc Pack(SarcData &data, s32 _align = -1);
	static SarcData Unpack(std::vector<u8> &data);

private:
	static u32 NameHash(const std::string &name);
	static u32 StringHashToUint(const std::string &name);
	static std::string GuessFileExtension(const std::vector<u8> &file);
	static u32 GuessAlignment(const std::unordered_map<std::string, std::vector<u8>> &files);
	static u32 GuessFileAlignment(const std::vector<u8> &file);
};