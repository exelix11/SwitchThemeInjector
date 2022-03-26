#include "UninstallPage.hpp"
#include "../ViewFunctions.hpp"
#include "../SwitchTools/PatchMng.hpp"

using namespace std;

UninstallPage::UninstallPage()
{
	Name = "Uninstall theme";
}

void UninstallPage::Render(int X, int Y)
{
	Utils::ImGuiSetupPage(this, X, Y);
	ImGui::PushFont(font30);

	ImGui::TextWrapped("Use this to uninstall the currently installed themes.\nIf you have issues, you can try removing the whole LayeredFS folder and code patches.");

	ImGui::PushStyleColor(ImGuiCol_Button, u32(0x6B70000ff));
	
	auto i = Utils::ImGuiCenterButtons({ "Uninstall the current theme", "Uninstall everything" } , &firstBtn);
	if (i != -1)
	{
		PushFunction([i]() {
			if (!YesNoPage::Ask("Are you sure ?")) return;
			if (i == 1)
			{
				DisplayLoading("Clearing LayeredFS dir...");
				fs::theme::UninstallTheme(true);
				PatchMng::RemoveAll();
				Dialog(
					"Done, everything theme-related has been removed, restart your console to apply the changes.\n"
					"As this removed the home menu patches as well you should restart this app before installing any theme."
				);
			}
			else
			{
				DisplayLoading("Loading...");
				fs::theme::UninstallTheme(false);
				Dialog("Done, all the installed themes have been removed, restart your console to apply the changes");
			}
		});
	}
	
	PAGE_RESET_FOCUS_FOR(firstBtn);
	ImGui::PopStyleColor();

	ImGui::PopFont();
	Utils::ImGuiSetWindowScrollable();
	Utils::ImGuiCloseWin();
}

void UninstallPage::Update()
{	
	if (Utils::PageLeaveFocusInput() && ImGui::GetFocusID() == firstBtn){
		Parent->PageLeaveFocus(this);
	}
}




