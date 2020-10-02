#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <streambuf>
#include "picosha2.h"

namespace Util
{	
	static inline std::string StringHash(const std::vector<unsigned char>& data)
	{
		auto h = picosha2::hash256_hex_string(data);

		for (char& c : h)
			c = std::toupper(c);

		return h;
	}

	static inline void WriteFile(const std::string& name, const std::vector<unsigned char>& data)
	{
		FILE* f = fopen(name.c_str(), "wb");
		if (!f)
			throw std::runtime_error("Saving file " + name + "failed !");

		fwrite(data.data(), 1, data.size(), f);
		fflush(f);
		fclose(f);
	}

	static inline std::vector<unsigned char> ReadData(const std::string& name)
	{
		FILE* f = fopen(("../Tests/Cases/" + name).c_str(), "rb");
		if (!f)
			throw std::runtime_error("Opening file " + name + " failed !\n");

		fseek(f, 0, SEEK_END);
		size_t len = 0;
		{
			auto fsz = ftell(f);
			if (fsz < 0)
				throw std::runtime_error("Reading file size for " + name + " failed !\n");
			len = fsz;
		}
		rewind(f);

		std::vector<unsigned char> coll(len);
		if (fread(coll.data(), 1, len, f) != len)
			throw std::runtime_error("Reading from file " + name + " failed !\n");

		fclose(f);
		return coll;
	}

	static inline std::string ReadString(const std::string& name)
	{
		std::ifstream t("../Tests/Cases/" + name);
		if (!t.good())
			throw std::runtime_error("");

		std::string str((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());
		return str;
	}

	static inline bool Exists(const std::string& name)
	{
		return std::filesystem::exists("../Tests/Cases/" + name);
	}
}