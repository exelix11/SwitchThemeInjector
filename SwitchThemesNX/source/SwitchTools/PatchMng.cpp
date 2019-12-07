#include "PatchMng.hpp"
#include <string>
#include "../fs.hpp"
#include "../SwitchThemesCommon/NXTheme.hpp"
#include <filesystem>
#include "../Platform/Platform.hpp"
#include <unordered_map>
#include "../UI/DialogPages.hpp"

using namespace std;

static const u32 PatchSetVer = 1;
#define LastSupportedVerSTR "9.0.1"
static const SystemVersion LastSupportedVer = { 9,0,1 };

#define ThemePatchesDir "NxThemesInstaller/"

#define WarnIntro "Since 9.0 some parts of the home menu require a custom code patch (exefs patch) to run properly.\n"
#define WarnOutro "\n\nWithout the correct patches some themes may crash, you will be warned when installing a theme that's known to cause issues"
const char* WarningCFW = WarnIntro "Unfortunately your cfw doesn't seem to suppot ips patches for titles." WarnOutro;
const char* WarningFWVer = 
	WarnIntro "You're running a newer firmware version that may be not supported by this installer (This build supports up to " LastSupportedVerSTR ").\n"
			  "If the home menu was updated it's likely that the built-in patches won't work, if that's the case you should check for updates" WarnOutro;
const char* WarningSDFail = WarnIntro "There was an error accessing the patches directory on your sd card, you could be affected by sd corruption (likely on exFat) or the archive bit issue." WarnOutro;

const char* PatchMng::InstallWarnStr = 
	"The theme you're trying to install is known to crash without an home menu patch and you don't seem to have a compatible one installed,"
	"it may work but it's possible that it will crash on boot. Do you want to continue ?\n\n"
	"In case of crash on boot you can delete the theme by manually removing the 0100000000001000 folder from /<your cfw>/titles on your sd card";

static const unordered_map<string, SystemVersion> PartsRequiringPatch = 
{
	{"Entrance.szs", {9,0,0} }
};

static bool HasLatestPatches = true;

static string GetExefsPatchesPath()
{
	if (fs::GetCfwFolder() == SD_PREFIX ATMOS_DIR)
		return SD_PREFIX ATMOS_DIR "/exefs_patches/";
	else if (fs::GetCfwFolder() == SD_PREFIX REINX_DIR)
		return SD_PREFIX REINX_DIR "/patches/";
	else return "";
}

bool PatchMng::CanInstallTheme(const string& FileName)
{
	if (HOSVer.major < 9) return true;
	if (!PartsRequiringPatch.count(FileName)) return true;
	
	const auto& ver = PartsRequiringPatch.at(FileName);

	if (HOSVer.IsGreater(ver) || HOSVer.IsEqual(ver))
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

const char* PatchMng::EnsureInstalled()
{
	if (HOSVer.major < 9) return nullptr;
	auto&& outDir = GetExefsPatchesPath();
	if (outDir == "")
	{
		HasLatestPatches = false;
		return WarningCFW;
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
		return WarningSDFail;

	if (HOSVer.IsGreater(LastSupportedVer)) {
		HasLatestPatches = false;
		return WarningFWVer;
	}

	return nullptr;
}

