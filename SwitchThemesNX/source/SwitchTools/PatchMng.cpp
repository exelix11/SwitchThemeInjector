#include "PatchMng.hpp"
#include <string>
#include "../fs.hpp"
#include "../SwitchThemesCommon/NXTheme.hpp"
#include <filesystem>
#include "../Platform/Platform.hpp"
#include <unordered_map>

using namespace std;

static const u32 PatchSetVer = 1;
#define LastSupportedVerSTR "9.0.0"
static const SystemVersion LastSupportedVer = { 9,0,0 };

const char* PatchMng::WarningStr =
	"Since 9.0 some parts of the home menu require a custom code patch (exefs patch) to run properly,"
	"if you're seeing this screen it means that these patches weren't applied correctly,"
	"this can happen because one of the following reasons:\n"
	" - Your CFW doesn't support ips patches, currently only Atmosphere and ReiNX do.\n"
	" - You're running a newer firmware, in this case check for updated.\n"
	"   - This version of the installer supports up to " LastSupportedVerSTR "\n"
	" - The patches directory on the sd card couldn't be written or read\n\n"
	"As the patches won't be loaded by your CFW some themes may crash, you will be warned when installing a theme that's known to cause issues";

const char* PatchMng::InstallWarnStr = 
	"The theme you're trying to install is known to not work without an home menu patch and you don't seem to have a compatible one installed,"
	"it may work but it's possible that it will crash on boot. Do you want to continue ?\n\n"
	"In case of crash on boot you can delete the theme by manually removing the 0100000000001000 folder from /<your cfw>/titles on your sd card";

static const unordered_map<string, SystemVersion> PartsRequiringPatch = 
{
	{"entrance.szs", {9,0,0} }
};

static bool HasLatestPatches = false;

static string GetExefsPatchesPath()
{
	if (CfwFolder == SD_PREFIX "/atmosphere")
		return SD_PREFIX "/atmosphere/exefs_patches/";
	else if (CfwFolder == SD_PREFIX "/reinx")
		return SD_PREFIX "/reinx/patches/";
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

void PatchMng::RemoveAll()
{
	fs::RecursiveDeleteFolder(GetExefsPatchesPath() + "NxThemesInstaller/");
	HasLatestPatches = false;
}

static bool ExtractPatches() 
{
	auto&& p = GetExefsPatchesPath();

	mkdir(p.c_str(), ACCESSPERMS);
	p += "NxThemesInstaller/";
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

	HasLatestPatches = true;
	return true;
}

bool PatchMng::EnsureInstalled()
{
	return false;

	if (HOSVer.major < 9) return true;
	auto&& outDir = GetExefsPatchesPath();
	if (outDir == "") return false;

	FILE* f = fopen((outDir + "ver.txt").c_str(), "r");
	if (!f)
		return ExtractPatches();

	u32 CurVer = 0;
	fscanf(f, "%u", &CurVer);
	fclose(f);

	if (CurVer < PatchSetVer)
		return ExtractPatches();

	if (HOSVer.IsGreater(LastSupportedVer))
		return false;

	HasLatestPatches = true;
	return true;
}

