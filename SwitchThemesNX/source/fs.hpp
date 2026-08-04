#pragma once
#include "SwitchThemesCommon/MyTypes.h"
#include <vector>
#include <string>
#include <span>

#include "Platform/PlatformFs.hpp"

bool StrEndsWith(const std::string &str, const std::string &suffix);
bool StrStartsWith(const std::string& str, const std::string& prefix);

namespace fs::path
{
	const std::string ThemesFolder = SD_PREFIX "/themes/";
	const std::string SystemDataFolder = SD_PREFIX "/themes/systemData/";
	const std::string DownloadsFolder = SD_PREFIX "/themes/Downloads/";
	const std::string ProvidersFile = SD_PREFIX "/themes/providers.json";
	const std::string PatchesDir = SD_PREFIX "/themes/systemPatches/";
	
	const std::string BootloaderDir = SD_PREFIX "/bootloader/";
	const std::string BootlogoPath = SD_PREFIX "/bootloader/bootlogo.bmp";

	const std::string& CfwFolder();
	std::string FsMitmFolder();
	std::string RomfsFolder(const std::string& contentID);

	std::string GetFreeDownloadFolder();

	// Modifies in-place
	std::string& ToUnixSeparators(std::string& str);

	const std::string Atmosphere = SD_PREFIX "/atmosphere/";
	const std::string Reinx = SD_PREFIX "/reinx/";
	const std::string SX = SD_PREFIX "/sxos/";
}

namespace fs {
	std::vector<u8> OpenFile(const std::string& name);
	
	void WriteFile(const std::string& name, std::span<const u8> data);

	bool Exists(const std::string& name);
	bool DirectoryExists(const std::string& name);
	void Delete(const std::string& path);
	void CreateDirectory(const std::string& path);
	void DeleteDirectory(const std::string& path);

	bool CheckFlagFile(const std::string& name);
	void SetFlagFile(const std::string& name, bool value);

	constexpr std::string GetFileName(const std::string& path)
	{
		return path.substr(path.find_last_of("/\\") + 1);
	}

	std::string GetPath(const std::string& path);
	std::string GetParentDir(const std::string& path);
	std::string JoinPath(const std::string& first, const std::string& second);

	// Meant for file names, truncates to 30 characters and replaces the following characters /?<>\:*|". with _
	// WIll remove file extensions
	std::string SanitizeName(const std::string& name);

	bool EnsureThemesFolderExists();
	void EnsureDownloadsFolderExists();
	void RemoveSystemDataDir();
}

namespace fs::cfw {
	bool IsAms();
	bool IsSX();
	bool IsRnx();

	std::vector<std::string> SearchFolders();
	void SetFolder(const std::string&);
}

namespace fs::patches {
	// These are not patches but just the strings
	std::vector<std::string> GetSdPatches();

	void CreateFolder();

	void WritePatchForBuild(const std::string& buildId, const std::vector<u8>&);
	std::vector<u8> OpenPatchForBuild(const std::string& buildId);
	bool hasPatchForBuild(const std::string& buildId);
}

namespace fs::theme {
	std::vector<std::string> ScanThemeFiles();
	void RequestThemeListRefresh();
	bool ShouldRescanThemeList();

	void UninstallTheme(bool full = false);
	void CreateMitmStructure(const std::string& id);
	void CreateRomfsDir(const std::string& id);
	void CreateStructure(const std::string& id);
	void WriteSystemVersionFile();
}