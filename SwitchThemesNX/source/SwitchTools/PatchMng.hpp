#pragma once
#include <string>

namespace PatchMng
{
	bool Init();

	const std::string& QlaunchBuildId();

	enum class InstallResult {
		Ok,
		MissingIps,
		SDError,
		UnsupportedCFW
	};

	InstallResult EnsureInstalled();
	void RemoveAll();

	bool CanInstallTheme(const std::string& FileName);
	bool ExefsCompatAsk(const std::string& SzsName);
};