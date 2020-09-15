#pragma once
#include "../fs.hpp"
#include <string>
#include <array>

namespace hactool {
	constexpr u64 QlaunchID = 0x0100000000001000;
	constexpr u64 PslID = 0x0100000000001007;
	constexpr u64 UserPageID = 0x0100000000001013;

	void ExtractPlayerSelectMenu();
	void ExtractUserPage();
	void ExtractHomeMenu();
	void ExtractTitle(u64 contentID, const std::string& Path);
	
	void ExtractHomeExefs();

	std::array<u8, 32> GetTitleBuildID(u64 contentID);
	std::string BuildIDToString(std::array<u8, 32> data);

	std::string QlaunchBuildID();
}