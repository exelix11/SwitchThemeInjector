#include "SettingsPage.hpp"
#include "../ViewFunctions.hpp"
#include "../Platform/Platform.hpp"

using namespace std;

namespace Settings {
	bool UseIcons = true;
	bool UseCommon = true;
	InstallCompatOption HomeMenuCompat = InstallCompatOption::Default;
};

SettingsPage::SettingsPage() 
{
	Name = "Settings";
}

void SettingsPage::Render(int X, int Y)
{
	Utils::ImGuiSetupWin(Name.c_str(), X, Y, DefaultWinFlags);
	ImGui::SetWindowSize(ImVec2(SCR_W - (float)X - 30, SCR_H - (float)Y - 70));
	ImGui::PushFont(font25);

	ImGui::PushFont(font30);
	ImGui::TextUnformatted("NXTheme settings");
	ImGui::PopFont();
	ImGui::TextWrapped("These settings only apply for installing nxthemes and are not saved, you have to switch them back every time you launch this app");
	ImGui::Checkbox("Enable custom icons", &Settings::UseIcons);
	PAGE_RESET_FOCUS;
	ImGui::Checkbox("Enable extra layouts (eg. common.szs)", &Settings::UseCommon);

	ImGui::Spacing();
	ImGui::Text("Home menu compatibility options.");
	ImGui::TextWrapped("Changing this could help solve install issues with old themes on latest firmware.");
	ImGui::RadioButton("Apply compatibility fixes (default)", (int*)&Settings::HomeMenuCompat, (int)Settings::InstallCompatOption::Default);
	ImGui::RadioButton("Always remove new applet icons", (int*)&Settings::HomeMenuCompat, (int)Settings::InstallCompatOption::ForceLegacyLayout);
	ImGui::RadioButton("Do not apply compatibility fixes", (int*)&Settings::HomeMenuCompat, (int)Settings::InstallCompatOption::BypassFixes);

	ImGui::NewLine();

	ImGui::PopFont();
	Utils::ImGuiCloseWin();
}

void SettingsPage::Update()
{	
	if (Utils::PageLeaveFocusInput(true))
	{
		Parent->PageLeaveFocus(this);
		return;
	}
}









