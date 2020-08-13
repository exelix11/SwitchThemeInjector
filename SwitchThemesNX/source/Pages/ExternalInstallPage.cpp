#include "ExternalInstallPage.hpp"
#include "../ViewFunctions.hpp"
#include "ThemeEntry/ThemeEntry.hpp"
#include "CfwSelectPage.hpp"
#include "../SwitchTools/PayloadReboot.hpp"
#include "../UI/UIManagement.hpp"

using namespace std;

ExternalInstallPage::ExternalInstallPage(const vector<string> &paths)
{
    for (int i=0; i < (int)paths.size(); i++)
    {
        ArgEntries.push_back(ThemeEntry::FromFile(paths[i]));
    }
}

ExternalInstallPage::~ExternalInstallPage()
{
	ArgEntries.clear();
}

void ExternalInstallPage::Render(int X, int Y)
{	
	Utils::ImGuiSetupWin("ExtInstallPage", 0, 0, DefaultWinFlags | ImGuiWindowFlags_NoBringToFrontOnFocus);
	ImGui::SetWindowSize({ SCR_W, SCR_H });
	ImGui::PushFont(font30);

	if (isInstalled)
	{
		ImGui::SetCursorPosY(80);

		if (!installSuccess)
			Utils::ImGuiCenterString("Theme(s) may have failed to install");
		else
			Utils::ImGuiCenterString("Installation completed.");

		ImGui::SetCursorPosY(SCR_H - 180);
		auto res = Utils::ImGuiCenterButtons({ "Exit to homebrew launcher" ,"Reboot" });
		Utils::ImGuiSelectItemOnce(true);
		if (res == 0)
		{
			App::Quit();
		}
		if (res == 1)
		{
			if (PayloadReboot::Init())
			{
				PayloadReboot::Reboot();
			}
			else
			{
#if __SWITCH__
				bpcInitialize();
				bpcRebootSystem();
#else
				App::Quit();
#endif
			}
		}
    }
	else
	{
		ImGui::SetCursorPosY(10);
		Utils::ImGuiCenterString("Install theme(s) from external source");

		ImGui::SetCursorPosY(SCR_H - 50);
		Utils::ImGuiCenterString("Press + to install, B to cancel");

		Utils::ImGuiSetupWin("ExtInstallPageContent", 20, 60, DefaultWinFlags & ~ImGuiWindowFlags_NoScrollbar);
		ImGui::SetWindowSize({ SCR_W - 20, SCR_H - 110 });
		for (int i=0; i < (int)ArgEntries.size(); i++)
        {
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ThemeEntry::EntryW / 2);
			if (ArgEntries[i]->Render() == ThemeEntry::UserAction::Preview)
				break;
			if (ImGui::IsItemActive())
			{
				auto drag = ImGui::GetMouseDragDelta(0);
				ImGui::SetScrollY(ImGui::GetScrollY() - drag.y);
			}
        }
		Utils::ImGuiSetWindowScrollable();
		Utils::ImGuiCloseWin();
    }

	ImGui::PopFont();
	Utils::ImGuiCloseWin();
}

void ExternalInstallPage::Update()
{
    if (isInstalled)
    {
        
    }
	else
    {		
        if (KeyPressed(GLFW_GAMEPAD_BUTTON_START))
        {
            DisplayLoading("Installing...");
            for (int i=0; i < (int)ArgEntries.size(); i++)
            {
                if(!ArgEntries[i]->Install(false)) installSuccess = false;
            }
			isInstalled = true;
        }
		else if (KeyPressed(GLFW_GAMEPAD_BUTTON_B))
		{
			App::Quit();
		}
    }
}




