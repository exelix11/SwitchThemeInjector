#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../UI/UI.hpp"
#include "../fs.hpp"

namespace Settings {
	enum class InstallCompatOption : int
	{
		Default,
		ForceLegacyLayout,
		BypassFixes
	};

	extern bool UseIcons;
	extern bool UseCommon;
	extern InstallCompatOption HomeMenuCompat;
};

class SettingsPage : public IPage
{
	public:
		SettingsPage();	
		
		void Render(int X, int Y) override;
		void Update() override;
};
