#include <array>
#include <string>
#include <unordered_map>
#include <format>
#include <vector>

#include "Common.hpp"
#include "MyTypes.h"
#include "../fs.hpp"

namespace SwitchThemesCommon 
{
	const std::string CoreVer = "4.9 (C++)";
	// Starting from v17 zip nxthemes are supported, before only szs were supported.
	const int NXThemeVer = 17;
}

namespace hos {
	SystemVersion Version = { 0,0,0 };
	std::array<u8, 0x40> VersionHash;
}

// Info for fw 6.0+
const std::unordered_map<std::string, ThemeTargetInfo> ThemeTargetList6
{
	{"home", { ThemeTargetInfo::QlaunchID  , "Home menu"        , "/lyt/ResidentMenu.szs" } },
	{"lock", { ThemeTargetInfo::QlaunchID  , "Lock screen"      , "/lyt/Entrance.szs" } },
	{"apps", { ThemeTargetInfo::QlaunchID  , "All apps menu"    , "/lyt/Flaunch.szs" } },
	{"set",	 { ThemeTargetInfo::QlaunchID  , "Settings applet"  , "/lyt/Set.szs" } },
	{"news", { ThemeTargetInfo::QlaunchID  , "News applet"      , "/lyt/Notification.szs" } },
	{"user", { ThemeTargetInfo::UserPageID , "User page"        , "/lyt/MyPage.szs" } },
	{"psl",	 { ThemeTargetInfo::PslID      , "Player selection" , "/lyt/Psl.szs" } },
};

// Info for fw <=6.x
const std::unordered_map<std::string, ThemeTargetInfo> ThemeTargetList5
{
	{"home", { ThemeTargetInfo::QlaunchID  , "Home menu"        , "/lyt/ResidentMenu.szs" } },
	{"lock", { ThemeTargetInfo::QlaunchID  , "Lock screen"      , "/lyt/Entrance.szs" } },
	// These behaved differently before 6.0 due to common.szs being used instead of resident menu
	{"apps", { ThemeTargetInfo::QlaunchID  , "All applets"      , "/lyt/Flaunch.szs" } },
	{"set",	 { ThemeTargetInfo::QlaunchID  , "All applets"      , "/lyt/Set.szs" } },
	{"news", { ThemeTargetInfo::QlaunchID  , "All applets"      , "/lyt/Notification.szs" } },
	{"user", { ThemeTargetInfo::UserPageID , "User page"        , "/lyt/MyPage.szs" } },
	{"psl",	 { ThemeTargetInfo::PslID      , "Player selection" , "/lyt/Psl.szs" } },
};

const ThemeTargetInfo ThemeTargetInfo::QlaunchCommon =
{
	QlaunchID, "Home menu common layout", "/lyt/common.szs"
};

std::string ThemeTargetInfo::TitleIdToString(u64 tid)
{
	return std::format("{:016x}", tid);
}

std::string ThemeTargetInfo::StringContentId() const
{
	return TitleIdToString(TitleId);
}

std::string_view ThemeTargetInfo::FindName(std::string nxPartName)
{
	if (nxPartName == ImagePartName)
		return "Image file";

	const auto info = Find(nxPartName);
	if (info)
		return info->PartName;

	return "Unknown part name";
}

const ThemeTargetInfo* ThemeTargetInfo::Find(std::string nxThemeName)
{
	// TODO: Do we even need to support older firmware anymore?
	if (hos::Version.major <= 5)
	{
		if (ThemeTargetList5.count(nxThemeName))
			return &ThemeTargetList5.at(nxThemeName);
	}
	else
	{
		if (ThemeTargetList6.count(nxThemeName))
			return &ThemeTargetList6.at(nxThemeName);
	}

	return nullptr;
}

std::vector<std::string> ThemeTargetInfo::GetTargetsForTitleId(u64 tid)
{
	std::vector<std::string> res = {};

	if (tid == QlaunchID)
		res.push_back(QlaunchCommon.SzsFile);

	// Only care about ThemeTargetList6 since the szs file names are the same for both
	for (const auto& [name, info] : ThemeTargetList6)
	{
		if (info.TitleId == tid)
			res.push_back(info.SzsFile);
	}

	return res;
}

const ThemeTargetInfo* ThemeTargetInfo::FindBySzsName(std::string szsName, std::string& outNxPartName)
{
	// Only care about ThemeTargetList6 since the szs file names are the same for both
	for (const auto& [name, info] : ThemeTargetList6)
	{
		if (fs::GetFileName(info.SzsFile) == szsName) {
			outNxPartName = name;
			return &info;
		}
	}

	outNxPartName = "";
	return nullptr;
}