#include <unordered_map>
#include <string>
#include "PatchMng.hpp"
#include "hactool.hpp"
#include "../fs.hpp"
#include "../SwitchThemesCommon/Common.hpp"
#include "../UI/DialogPages.hpp"

using namespace std;

namespace {
	const unordered_map<string, SystemVersion> PartsRequiringPatch =
	{
		{"Entrance.szs", {9,0,0} }
	};

	bool HasLatestPatches = false;

	std::string qlaunchBuildID;

	string GetExefsPatchesPath()
	{
		if (fs::cfw::IsAms())
			return fs::path::CfwFolder() + "exefs_patches/NxThemesInstaller/";
		else return "";
	}

	const char* InstallWarnStr =
		"Installing this theme will crash on boot because you do not have the needed home menu patches.\n"
		"To install patches refer to the main tab of the theme installer.\n\n"
		"To fix crashes you will need to manually delete the /atmosphere/contents/0100000000001000 folder from your SD card using a computer.\n\n"
		"Do you want to continue anyway?";

}; 

bool PatchMng::Init() {
	try
	{
		qlaunchBuildID = hactool::QlaunchBuildID();
		LOGf("Qlaunch build ID is %s\n", qlaunchBuildID.c_str());
	}
	catch (std::exception& ex)
	{
		LOGf("Qlaunch build ID error %s\n", ex.what());
		return false;
	}

	return true;
}

const std::string& PatchMng::QlaunchBuildId()
{
	return qlaunchBuildID;
}

bool PatchMng::CanInstallTheme(const string& FileName)
{
	if (hos::Version.major < 9) return true;
	if (!PartsRequiringPatch.count(FileName)) return true;
	
	const auto& ver = PartsRequiringPatch.at(FileName);

	if (hos::Version >= ver)
		return HasLatestPatches;
	else return true;

}

bool PatchMng::ExefsCompatAsk(const std::string& SzsName)
{
	if (!PatchMng::CanInstallTheme(SzsName))
		return YesNoPage::Ask(InstallWarnStr);
	return true;
}

void PatchMng::RemoveAll()
{
	fs::DeleteDirectory(GetExefsPatchesPath());
	fs::DeleteDirectory(fs::path::PatchesDir);
	HasLatestPatches = false;
}

PatchMng::InstallResult PatchMng::EnsureInstalled()
{
	if (hos::Version.major < 9) return InstallResult::Ok;

	auto exefsDir = GetExefsPatchesPath();
	if (exefsDir == "")
		return InstallResult::UnsupportedCFW;

	if (!fs::Exists(exefsDir))
		fs::CreateDirectory(exefsDir);

	if (qlaunchBuildID == "")
		return InstallResult::SDError;

	auto expectedPatchFile = exefsDir + qlaunchBuildID + ".ips";

	if (!fs::Exists(expectedPatchFile))
	{
		auto pathInRomfs = ASSET("patches/") + qlaunchBuildID + ".ips";

		try {
			if (fs::patches::hasPatchForBuild(qlaunchBuildID))
				fs::WriteFile(expectedPatchFile, fs::patches::OpenPatchForBuild(qlaunchBuildID));
			else if (fs::Exists(pathInRomfs))
				fs::WriteFile(expectedPatchFile, fs::OpenFile(pathInRomfs));
			else return InstallResult::MissingIps;
		}
		catch (...)
		{
			return InstallResult::SDError;
		}
	}
	
	HasLatestPatches = true;
	return InstallResult::Ok;
}

