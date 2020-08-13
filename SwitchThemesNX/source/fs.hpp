#pragma once
#include "SwitchThemesCommon/MyTypes.h"
#include <stdio.h>
#include <vector>
#include <string>
#include <filesystem>

#include "Platform/PlatformFs.hpp"

#define ATMOS_DIR "/atmosphere/"
#define REINX_DIR "/reinx/"
#define SX_DIR "/sxos/"

#define THEMES_DIR "themes"
#define THEMES_PATH SD_PREFIX "/" THEMES_DIR "/"
#define SYSTEMDATA_DIR "systemData"
#define SYSTEMDATA_PATH THEMES_PATH SYSTEMDATA_DIR "/"

bool StrEndsWith(const std::string &str, const std::string &suffix);
bool StrStartsWith(const std::string& str, const std::string& prefix);

namespace fs::path
{
	const std::string ThemesFolder = THEMES_PATH;
	const std::string SystemDataFolder = SYSTEMDATA_PATH;
	const std::string DownloadsFolder = THEMES_PATH "Downloads/";
	const std::string NcaVersionCfg = SYSTEMDATA_PATH "ver.cfg";

	std::string CfwFolder();
	std::string FsMitmFolder();
	std::string RomfsFolder(const std::string& contentID);

	std::string GetFreeDownloadFolder();

#ifdef __SWITCH__
	std::string Nca(u64 contentID);
#endif
}

namespace fs {
	void SetCfwFolder(const std::string&);

	std::vector<u8> OpenFile(const std::string& name);
	void WriteFile(const std::string& name, const std::vector<u8>& data);

	static inline bool Exists(const std::string& name) { return std::filesystem::exists(name); }
	static inline void Delete(const std::string& path) { unlink(path.c_str()); }
	static inline void CreateDirectory(const std::string& path) { mkdir(path.c_str(), ACCESSPERMS); }
	static inline void DeleteDirectory(const std::string& path) { rmdir(path.c_str()); }

	std::string GetFileName(const std::string& path);
	std::string GetPath(const std::string& path);
	std::string GetParentDir(const std::string& path);

	void RecursiveDeleteFolder(const std::string& path);

	// Meant for file names, truncates to 30 characters and replaces the following characters /?<>\:*|". with _
	// WIll remove file extensions
	std::string SanitizeName(const std::string& name);

	bool EnsureThemesFolderExists();
	void EnsureDownloadsFolderExists();
	void RemoveSystemDataDir();

	std::vector<std::string> SearchCfwFolders();
}

namespace fs::theme {
	std::vector<std::string> ScanThemeFiles();
	void RequestThemeListRefresh();
	bool ShouldRescanThemeList();

	void UninstallTheme(bool full = false);
	void CreateMitmStructure(const std::string& id);
	void CreateRomfsDir(const std::string& id);
	void CreateStructure(const std::string& id);

	bool DumpHomeMenuNca();
}