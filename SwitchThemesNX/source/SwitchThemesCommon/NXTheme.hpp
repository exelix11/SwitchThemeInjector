#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "MyTypes.h"
#include <unordered_map>
#include "SarcLib/Sarc.hpp"

struct ThemeFileManifest
{
	int Version;
	std::string Author;
	std::string ThemeName;
	std::string LayoutInfo;
	std::string Target;
};

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

struct SystemVersion { 
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

extern SystemVersion HOSVer;

extern std::unordered_map<std::string,std::string> ThemeTargetToName;
extern std::unordered_map<std::string,std::string> ThemeTargetToFileName;

const std::unordered_map<std::string,std::string> ThemeTargetToName6X
{
	{"home","Home menu"},
	{"lock","Lock screen"},
	{"user","User page"},
	{"apps","All apps menu"},
	{"set","Settings applet"},
	{"news","News applet" },
	{"psl","Player selection"},
};

const std::unordered_map<std::string,std::string> ThemeTargetToFileName6X
{
	{"home","ResidentMenu.szs"},
	{"lock","Entrance.szs"},
	{"user","MyPage.szs"},
	{"apps","Flaunch.szs"},
	{"set","Set.szs"},
	{"news","Notification.szs"},
	{"psl","Psl.szs" },
};

const std::unordered_map<std::string,std::string> ThemeTargetToName5X
{
	{"home","Home menu"},
	{"lock","Lock screen"},
	{"user","User page"},
	{"apps","All applets"},
	{"set","All applets"},
	{"news","All applets"},
	{"psl","Player selection" },
};

ThemeFileManifest ParseNXThemeFile(SARC::SarcData &SData);