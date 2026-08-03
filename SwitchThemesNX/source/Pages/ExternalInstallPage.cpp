#include "ExternalInstallPage.hpp"
#include "../ViewFunctions.hpp"
#include "ThemeEntry/ThemeEntry.hpp"
#include "CfwSelectPage.hpp"
#include "../UI/UIManagement.hpp"

using namespace std;

ExternalInstallPage::ExternalInstallPage(const vector<string>& paths)
{
	for (int i = 0; i < (int)paths.size(); i++)
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

	if (processStarted)
	{
		ImGui::SetCursorPosY(80);

		if (installationErrorMessage.size())
			Utils::ImGuiCenterString(installationErrorMessage);
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
			PlatformReboot();
		}
	}
	else
	{
		ImGui::SetCursorPosY(10);
		Utils::ImGuiCenterString("Install theme(s) from external source");

		ImGui::SetCursorPosY(SCR_H - 50);
		auto choice = Utils::ImGuiCenterButtons({ "Confirm install (+ button)", "Cancel" });
		if (choice == 0) installationRequested = true;
		else if (choice == 1) cancellationRequested = true;

		Utils::ImGuiSetupWin("ExtInstallPageContent", 20, 60, DefaultWinFlags & ~ImGuiWindowFlags_NoScrollbar);
		ImGui::SetWindowSize({ SCR_W - 20, SCR_H - 110 });
		for (int i = 0; i < (int)ArgEntries.size(); i++)
		{
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ThemeEntry::EntryW / 2);
			if (ArgEntries[i]->Render() == ThemeEntry::UserAction::Options)
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
	if (processStarted)
		return;

	if (KeyPressed(GLFW_GAMEPAD_BUTTON_START))
		installationRequested = true;
	if (KeyPressed(GLFW_GAMEPAD_BUTTON_B))
		cancellationRequested = true;

	if (installationRequested)
	{
		processStarted = true;

		DisplayLoading("Installing...");
		for (int i = 0; i < (int)ArgEntries.size(); i++)
		{
			if (!ArgEntries[i]->Install(false))
			{
				if (installationErrorMessage.empty())
					installationErrorMessage = "The following themes failed to install:\n";

				installationErrorMessage.append("- " + ArgEntries[i]->GetPath() + "\n");
			}
		}		
	}
	else if (cancellationRequested)
	{
		processStarted = true;
		App::Quit();
	}
}




