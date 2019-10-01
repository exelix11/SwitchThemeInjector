#pragma once
#include <string>

namespace PatchMng
{
	extern const char* InstallWarnStr;
	void RemoveAll();
	//returns an error message
	const char* EnsureInstalled();
	bool CanInstallTheme(const std::string& FileName);

	bool ExefsCompatAsk(const std::string& SzsName);
};