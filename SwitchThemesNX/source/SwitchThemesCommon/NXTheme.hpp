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

struct SystemVersion { 
	u32 major, minor, micro;
	bool IsGreater(const SystemVersion& other) const 
	{
		if (major > other.major)
			return true;
		else if (major < other.major)
			return false;
		else
		{
			if (minor > other.minor)
				return true;
			else if (minor < other.minor)
				return false;
			else return micro > other.micro;
		}
	}
	bool IsEqual(const SystemVersion& other) const
	{
		return other.major == major && other.minor == minor && other.micro == micro;
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
	{"opt","Options menu" },
	{"psl","Player selection" },
};

const std::unordered_map<std::string,std::string> ThemeTargetToFileName6X
{
	{"home","ResidentMenu.szs"},
	{"lock","Entrance.szs"},
	{"user","MyPage.szs"},
	{"apps","Flaunch.szs"},
	{"set","Set.szs"},
	{"news","Notification.szs"},
	//{"opt","Option.szs" },
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
	//{"opt","Options menu" },
	{"psl","Player selection" },
};

ThemeFileManifest ParseNXThemeFile(SARC::SarcData &SData);