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
	bool UseCommon5X;
};

extern std::unordered_map<std::string,std::string> ThemeTargetToName;
extern std::unordered_map<std::string,std::string> ThemeTargetToFileName;

const std::unordered_map<std::string,std::string> ThemeTargetToName6X
{
	{"home","Home menu"},
	{"lock","Lock screen"},
	{"user","User page"},
	{"apps","All apps menu"},
	{"set","Settings applet"},
	{"news","News applet"},
};

const std::unordered_map<std::string,std::string> ThemeTargetToFileName6X
{
	{"home","ResidentMenu.szs"},
	{"lock","Entrance.szs"},
	{"user","MyPage.szs"},
	{"apps","Flaunch.szs"},
	{"set","Set.szs"},
	{"news","Notification.szs"},
};

const std::unordered_map<std::string,std::string> ThemeTargetToName5X
{
	{"home","Home menu"},
	{"lock","Lock screen"},
	{"user","User page"},
	{"apps","all applets"},
	{"set","all applets"},
	{"news","all applets"},
};

const std::unordered_map<std::string,std::string> ThemeTargetToFileName5X
{
	{"home","ResidentMenu.szs"},
	{"lock","Entrance.szs"},
	{"user","MyPage.szs"},
	{"apps","common.szs"},
	{"set","common.szs"},
	{"news","common.szs"},
};

ThemeFileManifest ParseNXThemeFile(SARC::SarcData &SData);