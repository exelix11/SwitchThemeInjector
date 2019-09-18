#pragma once
#include <string>

namespace PatchMng
{
	extern const char* WarningStr;
	extern const char* InstallWarnStr;
	void RemoveAll();
	bool EnsureInstalled();
	bool CanInstallTheme(const std::string& FileName);
};