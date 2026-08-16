#pragma once
#include "../SwitchThemesCommon/Patcher.hpp"
#include "../UI/UI.hpp"
#include <optional>

namespace Settings {
	extern bool UseIcons;
	extern bool UseCommon;
	extern SwitchThemesCommon::LayoutCompatibilityOption HomeMenuCompat;
};

class SettingsPage : public IPage
{
	public:
		SettingsPage();	
		
		void Render(int X, int Y) override;
		void Update() override;

		static void ReloadSysmoduleInfo();
		static bool InstallSysmodule();
		static bool RemoveSysmodule(bool dialogs);
	private:
		static std::optional<int> sysmoduleInstalled;
		static std::optional<int> bundledSysmodule;

		void UISysmoduleAlreadyInstalled();
		void UISysmoduleBuildMissing();
		void UISysmoduleUpdateAvailable();
		void UISysmoduleNotInstalled();

		void UISetupFirstItem();
		u32 firstUiItem;
		bool focusItemPreventsLeft;
};
