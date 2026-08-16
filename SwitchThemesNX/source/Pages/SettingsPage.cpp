#include <vector>
#include "SettingsPage.hpp"
#include "../fs.hpp"
#include "../ViewFunctions.hpp"
#include "../SwitchThemesCommon/MyTypes.h"
#include <algorithm>

using namespace std;

namespace Settings {
	bool UseIcons = true;
	bool UseCommon = true;
	SwitchThemesCommon::LayoutCompatibilityOption HomeMenuCompat = SwitchThemesCommon::LayoutCompatibilityOption::Default;
};

namespace 
{
	const char* MarkerMagic = "THEME_SYMODULE:";

	int ExtractVersionFromSysmodule(span<const u8> data, int fallbackVersion)
	{
		std::basic_string<u8> marker = reinterpret_cast<const u8*>(MarkerMagic);
		auto found = std::search(data.begin(), data.end(), marker.begin(), marker.end());
		
		if (found == data.end())
			return fallbackVersion;

		auto marker_end = std::find(found, data.end(), '=');
		if (marker_end == data.end()) // Shouldn't happen, but just in case
			return fallbackVersion;

		std::string version_str(found + marker.size(), marker_end);
		return std::stoi(version_str);
	}
}

SettingsPage::SettingsPage()
{
	Name = "Settings";
	ReloadSysmoduleInfo();

	if (sysmoduleInstalled && bundledSysmodule && *sysmoduleInstalled < *bundledSysmodule)
		NotificationIcon = true;
	else
		NotificationIcon = !fs::CheckFlagFile("sysmodule2");
}

#define SYSMODULE_ID "00FF007468656D65"

std::optional<int> SettingsPage::sysmoduleInstalled;
std::optional<int> SettingsPage::bundledSysmodule;

void SettingsPage::ReloadSysmoduleInfo()
{
	if (fs::Exists(fs::path::FsMitmFolder() + SYSMODULE_ID "/exefs.nsp"))
	{
		try 
		{
			// Original version was 1 and did not have the marker tag, assume this is the case if we don't find it
			sysmoduleInstalled = ExtractVersionFromSysmodule(fs::OpenFile(fs::path::FsMitmFolder() + SYSMODULE_ID "/exefs.nsp"), 1);
		}
		catch (const std::exception& ex)
		{
			LOGf("Error reading sysmodule version: %s", ex.what());
			sysmoduleInstalled = std::nullopt;
		}
	}
	else
	{
		sysmoduleInstalled = std::nullopt;
	}

	if (fs::Exists(ASSET("sysmodule/ThemeSysmodule.nsp")))
	{
		try
		{
			bundledSysmodule = ExtractVersionFromSysmodule(fs::OpenFile(ASSET("sysmodule/ThemeSysmodule.nsp")), -1);
			
			// We should always know the version bundled with the installer.
			if (bundledSysmodule == -1)
				bundledSysmodule = std::nullopt;
		}
		catch (const std::exception& ex)
		{
			LOGf("Error reading bundled sysmodule version: %s", ex.what());
			bundledSysmodule = std::nullopt;
		}
	}
	else
	{
		bundledSysmodule = std::nullopt;
	}
}

bool SettingsPage::InstallSysmodule()
{
	if (!bundledSysmodule)
	{
		Dialog("Failed to install the sysmodule. This version of the theme installer was built without the sysmodule binary.");
		return false;
	}

	try 
	{
		fs::CreateDirectory(fs::path::FsMitmFolder() + SYSMODULE_ID "/flags");
		fs::WriteFile(fs::path::FsMitmFolder() + SYSMODULE_ID "/exefs.nsp", fs::OpenFile(ASSET("sysmodule/ThemeSysmodule.nsp")));
		fs::WriteFile(fs::path::FsMitmFolder() + SYSMODULE_ID "/toolbox.json", fs::OpenFile(ASSET("sysmodule/toolbox.json")));
		fs::WriteFile(fs::path::FsMitmFolder() + SYSMODULE_ID "/flags/boot2.flag", std::vector<u8>{ 'a' });
		
		ReloadSysmoduleInfo();
		Dialog("The sysmodule has been installed. Restart your console to apply the changes.");
		
		return true;
	}
	catch (const std::exception& ex)
	{
		Dialog("Error installing sysmodule: "s + ex.what());
		return false;
	}
}

bool SettingsPage::RemoveSysmodule(bool dialogs)
{
	auto path = fs::path::FsMitmFolder() + SYSMODULE_ID "/";

	try {
		if (fs::Exists(path))
			fs::DeleteDirectory(path);

		ReloadSysmoduleInfo();

		if (dialogs)
			Dialog("The sysmodule has been uninstalled. Restart your console to apply the changes.");

		return true;
	}
	catch (const std::exception& ex)
	{
		Dialog("Error uninstalling sysmodule: "s + ex.what());
		return false;
	}
}

void SettingsPage::UISysmoduleAlreadyInstalled() 
{
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::Highlight);
	ImGui::Text("The sysmodule v%d is currently installed.", *sysmoduleInstalled);
	ImGui::PopStyleColor();
	ImGui::SameLine();
	if (ImGui::Button("Uninstall"))
		PushFunction([]() { RemoveSysmodule(true); });
}

void SettingsPage::UISysmoduleUpdateAvailable() 
{
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::Highlight);
	ImGui::Text("The sysmodule v%d is currently installed. A new version is available.", *sysmoduleInstalled);
	ImGui::PopStyleColor();

	if (ImGui::Button("Uninstall"))
		PushFunction([]() { RemoveSysmodule(true); });
	
	// Cursor hack, this should be the first item
	UISetupFirstItem();

	ImGui::SameLine();

	// TODO: actually due to imgui handling events together as our tab manager using the dpan right input to switch to the settings page will focus this instead
	// It's an edge case and fixing it is probably more trouble than it's worth.
	if (ImGui::Button("Update now"))
		PushFunction([]() { InstallSysmodule(); });

	focusItemPreventsLeft = ImGui::GetFocusID() == ImGui::GetCurrentWindow()->DC.LastItemId;
}

void SettingsPage::UISysmoduleBuildMissing()
{
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::Red);
	ImGui::TextWrapped("This version of the theme installer does not contain the sysmodule binary.");
	ImGui::PopStyleColor();
}

void SettingsPage::UISysmoduleNotInstalled() 
{
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::Red);
	ImGui::Text("The sysmodule is currently not installed.");
	ImGui::PopStyleColor();

	ImGui::SameLine();
	if (ImGui::Button("Install now"))
		PushFunction([]() { InstallSysmodule(); });
}

void SettingsPage::UISetupFirstItem()
{
	if (firstUiItem == 0)
		firstUiItem = ImGui::GetCurrentWindow()->DC.LastItemId;

	ImGui::SetItemDefaultFocus();
}

void SettingsPage::Render(int X, int Y)
{
	firstUiItem = 0;
	focusItemPreventsLeft = false;

	if (NotificationIcon)
	{
		NotificationIcon = false;
		fs::SetFlagFile("sysmodule2", true);
	}

	Utils::ImGuiSetupWin(Name.c_str(), X, Y, DefaultWinFlags);
	ImGui::SetWindowSize(ImVec2(SCR_W - (float)X - 30, SCR_H - (float)Y - 70));
	ImGui::PushFont(font25);

	ImGui::PushFont(font30);
	ImGui::TextUnformatted("Update detection sysmodule v2 (BETA)");
	ImGui::PopFont();

	ImGui::TextWrapped("This is a sysmodule that automatically uninstalls themes when the system firmware is updated. This fixes crashes after updates or when switching emummc.\nVersion 2 fixes a number of bugs reported by users.");
	if (sysmoduleInstalled)
	{
		if (!bundledSysmodule)
		{
			UISysmoduleBuildMissing();
			UISysmoduleAlreadyInstalled();
		}
		else if (*sysmoduleInstalled < *bundledSysmodule)
			UISysmoduleUpdateAvailable();
		else
			UISysmoduleAlreadyInstalled();
	}
	else if (bundledSysmodule)
		UISysmoduleNotInstalled();
	else
		UISysmoduleBuildMissing();
	
	UISetupFirstItem();

	PAGE_RESET_FOCUS_FOR(firstUiItem);

	// Fix scrolling: when the first item gets focused return at the top
	if (ImGui::GetScrollY() > 0 && ImGui::GetFocusID() == firstUiItem)
		ImGui::SetScrollY(0);

	ImGui::NewLine();

	ImGui::PushFont(font30);
	ImGui::TextUnformatted("NXTheme settings");
	ImGui::PopFont();

	ImGui::TextWrapped("These settings only apply for installing nxthemes and are not saved, you have to switch them back every time you launch this app");
	ImGui::Checkbox("Enable custom icons", &Settings::UseIcons);
	ImGui::Checkbox("Enable extra layouts (eg. common.szs)", &Settings::UseCommon);

	ImGui::NewLine();
	ImGui::Text("Home menu compatibility options.");
	ImGui::TextWrapped("Changing this could help solve install issues with old themes on latest firmware.");
	ImGui::RadioButton("Decide automatically (default)", (int*)&Settings::HomeMenuCompat, (int)SwitchThemesCommon::LayoutCompatibilityOption::Default);
	ImGui::RadioButton("Force original home menu applet icons (firmware <= 10.0)", (int*)&Settings::HomeMenuCompat, (int)SwitchThemesCommon::LayoutCompatibilityOption::Firmware10);
	ImGui::RadioButton("Force home menu layout with the NS online icon (firmware 11.0)", (int*)&Settings::HomeMenuCompat, (int)SwitchThemesCommon::LayoutCompatibilityOption::Firmware11);
	ImGui::RadioButton("Do not apply compatibility fixes", (int*)&Settings::HomeMenuCompat, (int)SwitchThemesCommon::LayoutCompatibilityOption::DisableFixes);

	ImGui::NewLine();

	ImGui::PopFont();
	Utils::ImGuiSetWindowScrollable();
	Utils::ImGuiCloseWin();
}

void SettingsPage::Update()
{
	if (Utils::PageLeaveFocusInput(!focusItemPreventsLeft))
	{
		Parent->PageLeaveFocus(this);
		return;
	}
}









