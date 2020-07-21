#include "SettingsPage.hpp"
#include "../ViewFunctions.hpp"
#include "../Platform/Platform.hpp"

using namespace std;

namespace Settings {
	bool UseAnimations = true;
	bool UseIcons = true;
	bool UseCommon = true;
};

SettingsPage::SettingsPage() 
{
	Name = "Settings";
}

void SettingsPage::Render(int X, int Y)
{
	Utils::ImGuiSetupWin(Name.c_str(), X, Y, DefaultWinFlags & ~ImGuiWindowFlags_NoScrollbar);
	ImGui::SetWindowSize(ImVec2(SCR_W - (float)X - 30, SCR_H - (float)Y - 70));
	ImGui::PushFont(font25);

	ImGui::PushFont(font30);
	ImGui::TextUnformatted("NXTheme settings");
	ImGui::PopFont();
	ImGui::TextWrapped("These settings only apply for installing nxthemes and are not saved, you have to switch them back every time you launch this app");
	ImGui::Checkbox("Enable animations", &Settings::UseAnimations);
	if (ImGui::IsItemFocused())
		ImGui::SetScrollY(0);
	ImGui::Checkbox("Enable custom icons", &Settings::UseIcons);
	ImGui::Checkbox("Enable extra layouts (eg. common.szs)", &Settings::UseCommon);
	PAGE_RESET_FOCUS
	ImGui::NewLine();

	ImGui::PopFont();
	Utils::ImGuiSetWindowScrollable();
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









