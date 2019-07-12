#include "ExternalInstallPage.hpp"
#include "../ViewFunctions.hpp"
#include "ThemeEntry.hpp"
#include "CfwSelectPage.hpp"
#include "../SwitchTools/PayloadReboot.hpp"
#include "../UI/UIManagement.hpp"

using namespace std;

ExternalInstallPage::ExternalInstallPage(const vector<string> &paths) :
Title("Install theme from external source"),
Install("Press + to install, B to cancel")
{
    for (int i=0; i < (int)paths.size(); i++)
    {
        this->ArgEntries.push_back(new ThemeEntry(paths[i]));
    }
}

ExternalInstallPage::~ExternalInstallPage()
{
	for (int i=0; i < (int)ArgEntries.size(); i++)
			delete ArgEntries[i];
	ArgEntries.clear();
}

void ExternalInstallPage::Render(int X, int Y)
{	
	Utils::ImGuiSetupWin("ExtInstallPage", 0, 0);
	ImGui::SetWindowSize({ SCR_W, SCR_H });
	ImGui::PushFont(font30);

	ImGui::SetCursorPosY(30);
	Utils::ImGuiCenterString(Title);

	if (isInstalled)
	{
		Utils::ImGuiCenterString(Title.c_str());
		if (ImGui::Button("Exit to homebrew launcher"))
		{
			SetAppShouldClose();
		}
		ImGui::SameLine();
		if (ImGui::Button("Reboot"))
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
				SetAppShouldClose();
#endif
			}
		}
    }else
    {
		Utils::ImGuiCenterString(Install);
		ImGui::SetCursorPos({ SCR_W / 2 - ThemeEntry::EntryW / 2 , 70 });
        for (int i=0; i < (int)ArgEntries.size(); i++)
        {
			if (ArgEntries[i]->Render() == ThemeEntry::UserAction::Preview)
				break;
        }
    }

	ImGui::PopFont();
	Utils::ImGuiSetWindowScrollable();
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
            bool installSuccess = true;
            for (int i=0; i < (int)ArgEntries.size(); i++)
            {
                if(!ArgEntries[i]->InstallTheme(false)) installSuccess = false;
            }
            if(!installSuccess)
            {
                Title = ("Theme(s) may have failed to install");
            }else{
                Title = "Installation completed.";
            }
			isInstalled = true;
        }
		else if (KeyPressed(GLFW_GAMEPAD_BUTTON_B))
		{
			SetAppShouldClose();
		}
    }
}




