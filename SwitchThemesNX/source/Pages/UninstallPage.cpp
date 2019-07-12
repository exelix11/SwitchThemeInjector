#include "UninstallPage.hpp"
#include "../ViewFunctions.hpp"

using namespace std;

UninstallPage::UninstallPage() : 
lblText("Use this to uninstall the currently installed themes.\nIf you have issues, try fully removing the LayeredFS directory by pressing L+R as well.")
{
	Name = "Uninstall theme";
}

void UninstallPage::Render(int X, int Y)
{
	Utils::ImGuiSetupPage("Uninstall page", X, Y, focused);
	ImGui::PushFont(font30);

	ImGui::TextWrapped(lblText.c_str());

	ImGui::PushStyleColor(ImGuiCol_Button, 0x6B70000ff);
	if (ImGui::Button("Uninstall"))
	{
		PushFunction([]() {
			if (!YesNoPage::Ask("Are you sure ?")) return;
			if (gamepad.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] && gamepad.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER])
			{
				DisplayLoading("Clearing LayeredFS dir...");
				UninstallTheme(true);
				Dialog("Done, the layeredFS dir of the home menu was removed, restart your console to see the changes");
			}
			else
			{
				DisplayLoading("Loading...");
				UninstallTheme(false);
				Dialog("Done, all the installed themes have been removed, restart your console to see the changes");
			}
		});
	}
	PAGE_RESET_FOCUS
	ImGui::PopStyleColor();

	ImGui::PopFont();
	Utils::ImGuiSetWindowScrollable();
	Utils::ImGuiCloseWin();
}

void UninstallPage::Update()
{	
	if (Utils::PageLeaveFocusInput()){
		Parent->PageLeaveFocus(this);
	}
}




