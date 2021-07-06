#include "PatchMng.hpp"
#include <string>
#include "../fs.hpp"
#include "../SwitchThemesCommon/NXTheme.hpp"
#include <filesystem>
#include "../Platform/Platform.hpp"
#include <unordered_map>
#include "../UI/DialogPages.hpp"
#include "hactool.hpp"
#include "../Dialogs.hpp"

using namespace std;

static const u32 PatchSetVer = 7;
#define LastSupportedVerSTR "12.1"

#define ThemePatchesDir "NxThemesInstaller/"

#define WarnIntro "Since 9.0 some parts of the home menu require a custom code patch (exefs patch) to run properly.\n"
#define WarnOutro "\n\nWithout the correct patches you can still install themes but some may crash, you will be warned when installing a theme that's known to cause issues"

//Is there even another CFW ?
const char* WarningCFW = WarnIntro "Unfortunately your CFW doesn't seem to suppot ips patches for titles." WarnOutro;

static const char* WarningSX = WarnIntro
		"\nIt seems you're using SX OS, support for these patches has been added only in version 2.9.4 beta.\n"
		"This means that if you're running an older version your CFW is not compatible, you're seeing this warning because this application cannot detect which is your current version.\n"
		"When installing a lockscreen theme you will be warned about missing patches, if you know for sure that you have a supported version you can safely install the theme.\n\n"
		"In case you don't have the right version and install the theme anyway your console will crash on boot, the warning before install also displays the instructions to fix it.";

static const char* WarningMissingPatch = WarnIntro
	 "You're running a firmware version that is not supported by this installer. The latest firmware at the time of this release is " LastSupportedVerSTR ".\n"
	 "The home menu has been updated and there's no matching patch, please check for updates on github." WarnOutro;

static const char* WarningSDFail = WarnIntro "There was an error accessing the patches directory on your sd card, you could be affected by sd corruption (likely on exFat) or the archive bit issue." WarnOutro;

static const char* ErrorHactool = "Couldn't detect the home menu version you're running. Hactool failed to extract the home menu version info. Please open an issue on github.";

const char* PatchMng::InstallWarnStr = 
	"The theme you're trying to install is known to crash without an home menu patch and you don't seem to have a compatible one installed,"
	"it may work but it's possible that it will crash on boot. Do you want to continue ?\n\n"
	"In case of crash on boot you can delete the theme by manually removing the 0100000000001000 folder from /atmosphere/contents on your sd card (/<your cfw>/titles for old atmosphere and other CFWs)";

static const unordered_map<string, SystemVersion> PartsRequiringPatch = 
{
	{"Entrance.szs", {9,0,0} }
};

static bool HasLatestPatches = true;

static string GetExefsPatchesPath()
{
	if (fs::cfw::IsAms() || fs::cfw::IsSX())
		return fs::path::CfwFolder() + "exefs_patches/";
	else if (fs::cfw::IsRnx())
		return fs::path::CfwFolder() + "patches/";
	else return "";
}

bool PatchMng::CanInstallTheme(const string& FileName)
{
	if (HOSVer.major < 9) return true;
	if (!PartsRequiringPatch.count(FileName)) return true;
	
	const auto& ver = PartsRequiringPatch.at(FileName);

	if (HOSVer >= ver)
		return HasLatestPatches;
	else return true;

}

bool PatchMng::ExefsCompatAsk(const std::string& SzsName)
{
	if (!PatchMng::CanInstallTheme(SzsName))
		return YesNoPage::Ask(PatchMng::InstallWarnStr);
	return true;
}

void PatchMng::RemoveAll()
{
	fs::RecursiveDeleteFolder(GetExefsPatchesPath() + ThemePatchesDir);
	HasLatestPatches = false;
}

static bool ExtractPatches() 
{
	auto&& p = GetExefsPatchesPath();

	mkdir(p.c_str(), ACCESSPERMS);
	p += ThemePatchesDir;
	mkdir(p.c_str(), ACCESSPERMS);

	try {
		for (const auto& v : filesystem::directory_iterator(ASSET("patches")))
			fs::WriteFile(p + v.path().filename().string(), fs::OpenFile(v.path().string()));
	}
	catch (...)
	{
		return false;
	}

	FILE* f = fopen((p + "ver.txt").c_str(), "w");
	if (!f)
		return false;
	fprintf(f, "%u", PatchSetVer);
	fclose(f);

	return true;
}

PatchMng::ErrorPage PatchMng::EnsureInstalled()
{
	if (HOSVer.major < 9) return { };

	auto&& outDir = GetExefsPatchesPath();
	if (outDir == "")
	{
		HasLatestPatches = false;
		return {"Warning", WarningCFW };
	}

	FILE* f = fopen((outDir + ThemePatchesDir "ver.txt").c_str(), "r");
	if (!f)
		HasLatestPatches = ExtractPatches();
	else {
		u32 CurVer = 0;
		fscanf(f, "%u", &CurVer);
		fclose(f);

		if (CurVer < PatchSetVer)
			HasLatestPatches = ExtractPatches();
	}
	if (!HasLatestPatches)
		return { "Warning" , WarningSDFail };

	try 
	{
		auto id = hactool::QlaunchBuildID();
		LOGf("Qlaunch build ID is %s\n", id.c_str());

		if (!fs::Exists(std::string(ASSET("patches/")) + id + ".ips"))
			return { "Warning" , WarningMissingPatch };
	}	
	catch (std::exception& ex)
	{
		DialogBlocking(ex.what());
		return { "Error", ErrorHactool };
	}

	if (fs::path::CfwFolder() == SD_PREFIX SX_DIR)
	{
		HasLatestPatches = false;
		return { "Warning", WarningSX };
	}

	return {};
}

