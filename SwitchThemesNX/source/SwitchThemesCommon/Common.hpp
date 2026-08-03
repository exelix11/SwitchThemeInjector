#pragma once
#include <string>
#include <array>
#include <vector>
#include <unordered_map>
#include "MyTypes.h"

using FileData = std::vector<u8>;
using FileContainer = std::unordered_map<std::string, FileData>;

namespace SwitchThemesCommon
{
	// This is the C++ implementation of SwitchThemesCommon. While this is the one that runs on switch the main one to be used as a reference and for prototyping is the C# one.
	// The C# version also has better comments on the rationale behind things like compatibility fixes and patch ordering.
	extern const std::string CoreVer;
	extern const int NXThemeVer;
}

// This enum defines the compatibility level of layouts, it is not meant to map exactly to HOS versions. New versions are only added when there are breaking changes to address via the NewFirmFixes feature
enum class ConsoleFirmware : int
{
	// Default value
	Invariant = 0,
	// Firmware versions in the format A.B.C => A_B_C
	// These should be set in a way that makes them chronologically comparable with < and > operators
	Fw5_0 = 5'0'0,
	Fw6_0 = 6'0'0,
	Fw8_0 = 8'0'0,
	Fw9_0 = 9'0'0,
	Fw11_0 = 11'0'0,
	Fw20_0 = 20'0'0,
};

struct SystemVersion
{
	u32 major, minor, micro;

	constexpr auto operator<=>(const SystemVersion& other) const
	{
		auto m = major <=> other.major;
		if (m == std::strong_ordering::equal)
			m = minor <=> other.minor;
		if (m == std::strong_ordering::equal)
			m = micro <=> other.micro;
		return m;
	}

	ConsoleFirmware ToFirmwareEnum() const
	{
		if (major < 5) return ConsoleFirmware::Invariant;
		if (major == 5) return ConsoleFirmware::Fw5_0;
		if (major == 6 || major == 7) return ConsoleFirmware::Fw6_0;
		if (major == 8) return ConsoleFirmware::Fw8_0;
		if (major == 9 || major == 10) return ConsoleFirmware::Fw9_0;
		if (major >= 11 && major < 20) return ConsoleFirmware::Fw11_0;
		if (major >= 20) return ConsoleFirmware::Fw20_0;

		return ConsoleFirmware::Invariant;
	}
};

struct ThemeTargetInfo
{
	u64 TitleId;
	std::string PartName;
	std::string SzsFile;

	std::string StringContentId() const;

	static constexpr u64 QlaunchID = 0x0100000000001000;
	static constexpr u64 PslID = 0x0100000000001007;
	static constexpr u64 UserPageID = 0x0100000000001013;

	// Not part of target names but needed for extraction
	static const ThemeTargetInfo QlaunchCommon;

	// May be null if part name is not valid
	static const ThemeTargetInfo* Find(std::string nxThemeName);
	static const ThemeTargetInfo* FindBySzsName(std::string szsName, std::string& outNxPartName);

	static std::vector<std::string> GetTargetsForTitleId(u64 tid);
	static std::string TitleIdToString(u64 tid);
};

namespace hos 
{
	extern SystemVersion Version;
	extern std::array<u8, 0x40> VersionHash;
}