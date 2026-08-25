#include <string>
#include <exception>
#include "UninstallPage.hpp"
#include "SettingsPage.hpp"
#include "../ViewFunctions.hpp"
#include "../SwitchTools/PatchMng.hpp"
#include "../fs.hpp"

namespace 
{
	bool RemoveTheme(bool full) 
	{
		try 
		{
			fs::theme::UninstallTheme(full);
		}
		catch (const std::exception& ex)
		{
			Dialog("Error uninstalling theme: " + std::string(ex.what()));
			return false;
		}
		return true;
	}

	bool RemovePatches() 
	{
		try
		{
			PatchMng::RemoveAll();
		}
		catch (const std::exception& ex)
		{
			Dialog("Error while removing lockscreen patches: " + std::string(ex.what()));
			return false;
		}
		return true;
	}

	void RemoveHekateBootLogo()
	{
		if (!fs::Exists(fs::path::BootlogoPath))
		{
			Dialog("No Hekate boot image is installed.");
			return;
		}

		if (!YesNoPage::Ask("Remove /bootloader/bootlogo.bmp?"))
			return;

		try
		{
			fs::Delete(fs::path::BootlogoPath);
			Dialog("The Hekate boot image has been removed.");
		}
		catch (const std::exception& ex)
		{
			Dialog("Error removing the Hekate boot image: " + std::string(ex.what()));
		}
	}
}

UninstallPage::UninstallPage()
{
	Name = "Uninstall theme";
}

void UninstallPage::Render(int X, int Y)
{
	Utils::ImGuiSetupPage(this, X, Y);
	ImGui::PushFont(font30);

	ImGui::TextWrapped("Use this to uninstall the currently installed themes.\nIf you are facing issues, you can try removing the whole LayeredFS folder and code patches.");

	ImGui::PushStyleColor(ImGuiCol_Button, u32(0x6B70000ff));
	
	auto i = Utils::ImGuiCenterButtons({ "Uninstall the current theme", "Uninstall everything" } , &firstBtn);
	if (i != -1)
	{
		PushFunction([i]() {
			if (!YesNoPage::Ask("Are you sure ?")) return;
			if (i == 1)
			{
				DisplayLoading("Clearing custom themes data...");

				bool success = SettingsPage::RemoveSysmodule(false);
				success &= RemoveTheme(true);
				success &= RemovePatches();
				
				if (success) 
				{
					Dialog(
						"Done, everything theme-related has been removed, restart your console to apply the changes.\n"
						"As this removed the home menu patches as well you should restart this app before installing any theme."
					);
				}
			}
			else
			{
				DisplayLoading("Loading...");

				if (RemoveTheme(false))
					Dialog("Done, all the installed themes have been removed, restart your console to apply the changes");
			}
		});
	}

	ImGui::Spacing();
	Utils::ImGuiCenterString("Boot image");
	ImGui::Spacing();
	if (Utils::ImGuiCenterButton("Uninstall Hekate boot image"))
		PushFunction([]() { RemoveHekateBootLogo(); });
	
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
