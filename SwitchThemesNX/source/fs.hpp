#pragma once
#include "SwitchThemesCommon/MyTypes.h"
#include <stdio.h>
#include <vector>
#include <string>

#if WIN32
#include <io.h>
#include <direct.h>
#include "Platform\Windows\dirent.h"
#define mkdir(x,y) _mkdir(x)
#define rmdir(x) _rmdir(x)
#else
#include <unistd.h>
#include <dirent.h>
#include <switch.h>
#endif

#define ATMOS_DIR "/atmosphere/"
#define REINX_DIR "/reinx/"
#define SX_DIR "/sxos/"

bool StrEndsWith(const std::string &str, const std::string &suffix);
bool StrStartsWith(const std::string& str, const std::string& prefix);

namespace fs {
	std::string GetCfwFolder();
	std::string GetFsMitmFolder();
	void SetCfwFolder(const std::string&);

	std::vector<u8> OpenFile(const std::string& name);
	void WriteFile(const std::string& name, const std::vector<u8>& data);

	bool CheckThemesFolder();
	std::vector<std::string> SearchCfwFolders();
	std::vector<std::string> GetThemeFiles();

	void UninstallTheme(bool full = false);
	void CreateFsMitmStructure(const std::string& tid);
	void CreateRomfsDir(const std::string& tid);
	void CreateThemeStructure(const std::string& tid);
	
	std::string GetFileName(const std::string& path);
	std::string GetPath(const std::string& path);
	std::string GetParentDir(const std::string& path);

	void RecursiveDeleteFolder(const std::string& path);

	void RemoveSystemDataDir();
	bool WriteHomeDumpVer();
	
#ifdef __SWITCH__
	std::string GetNcaPath(u64 tid);
#endif
	bool DumpHomeMenuNca();
}