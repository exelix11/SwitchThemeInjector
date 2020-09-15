#pragma once
#include <string>

namespace PatchMng
{
	extern const char* InstallWarnStr;

	struct ErrorPage {
		const char *Title, *Content;

		operator bool() const { return Title && Content; }
	};

	ErrorPage EnsureInstalled();
	void RemoveAll();

	bool CanInstallTheme(const std::string& FileName);
	bool ExefsCompatAsk(const std::string& SzsName);
};