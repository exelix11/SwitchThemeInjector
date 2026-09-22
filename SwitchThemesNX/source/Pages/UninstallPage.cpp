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

	bool RemoveBootlogo()
	{
		try
		{
			if (fs::Exists(fs::path::BootlogoPath))
				fs::Delete(fs::path::BootlogoPath);
		}
		catch (const std::exception& ex)
		{
			Dialog("Error removing boot logo file: " + std::string(ex.what()));
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
}

UninstallPage::UninstallPage()
{
	Name = "Uninstall theme";
}

void UninstallPage::Render(int X, int Y)
{
	Utils::ImGuiSetupPage(this, X, Y);
	ImGui::PushFont(font30);

	ImGui::TextWrapped("Use these options to uninstall the currently installed themes.");

	ImGui::PushStyleColor(ImGuiCol_Button, u32(0x5B700009f));

	if (Utils::ImGuiCenterButton("Remove the current theme", 550))
	{
		PushFunction([]() {
			if (!YesNoPage::Ask(
				"This will remove the installed theme for all the supported home menu parts.\n"
				"Do you want to continue ?"
			))
				return;

			DisplayLoading("Loading...");

			if (RemoveTheme(false))
				Dialog("Done, all the installed themes have been removed, restart your console to apply the changes");
		});
	}
	PAGE_RESET_FOCUS;

	if (Utils::ImGuiCenterButton("Remove the hekate boot logo", 550))
	{
		PushFunction([]() {
			if (!YesNoPage::Ask(
				"This will remove the custom hekate boot screen.\n"
				"Do you want to continue ?"
			))
				return;

			if (RemoveBootlogo())
				Dialog("The hekate boot logo has been removed, restart your console to apply the changes");
		});
	}

	ImGui::NewLine();
	ImGui::TextWrapped("In case you're facing unexpected crashes, use the following option to remove everything related to themes including fsmitm folders, the version check sysmodule and theme patches.");

	if (Utils::ImGuiCenterButton("Uninstall everything", 550))
	{
		PushFunction([]() {
			if (!YesNoPage::Ask(
				"This will remove everything related to custom themes, such as:\n"
				"- The currently installed theme\n"
				"- Applied theme patches\n"
				"- The theme update check sysmodule\n"
				"- Any custom hekate boot logo\n\n"
				"Do you want to continue ?"
			))
				return;

			DisplayLoading("Clearing custom themes data...");

			bool success = SettingsPage::RemoveSysmodule(false);
			success &= RemoveTheme(true);
			success &= RemovePatches();
			success &= RemoveBootlogo();

			if (success)
			{
				Dialog(
					"Done, everything theme-related has been removed, restart your console to apply the changes.\n"
					"This removed any home menu patches as well, you should restart this app before installing themes again."
				);
			}
		});
	}

	ImGui::PopStyleColor();

	ImGui::PopFont();
	Utils::ImGuiSetWindowScrollable();
	Utils::ImGuiCloseWin();
}

void UninstallPage::Update()
{
	if (Utils::PageLeaveFocusInput()) {
		Parent->PageLeaveFocus(this);
	}
}




