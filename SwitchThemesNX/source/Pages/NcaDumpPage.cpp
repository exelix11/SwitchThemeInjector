#include "NcaDumpPage.hpp"
#include "../ViewFunctions.hpp"
#include "../fs.hpp"
#include "../SwitchTools/hactool.hpp"
#include <filesystem>
#include "../Platform/Platform.hpp"
#include "../SwitchThemesCommon/NXTheme.hpp"

using namespace std;

NcaDumpPage::NcaDumpPage()
{
	Name = "Extract home menu";
}

void NcaDumpPage::Render(int X, int Y)
{
	Utils::ImGuiSetupPage(this, X, Y);
	ImGui::PushFont(font30);

	ImGui::TextWrapped("To install .nxtheme files you need to extract the home menu first.\n"
		"This is needed every time the firmware changes, both for updates and downgrades.\n"
		"When the extracted version doesn't match with your firmware you will be prompted to do it.\n\n"
		"Usually you don't need to extract it manually but in case you're facing issues you can try doing so here.");

	if (ImGui::Button("Extract home menu"))
	{
		PushFunction([]() {
			if ((gamepad.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] && gamepad.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER]))
			{
				DialogBlocking("Super secret combination entered, only the home menu NCA will be dumped (it won't be extracted)");
				DisplayLoading("Extracting NCA...");
				if (fs::theme::DumpHomeMenuNca())
					Dialog("The home menu NCA was extracted, now use the injector to complete the setup.\nIf you didn't do this on purpose ignore this message.");
				return;
			}
			if (!YesNoPage::Ask(
				"To install custom themes you need to extract the home menu first, this process may take several minutes, don't let your console go to sleep mode and don't press the home button.\n"
				"Do you want to continue ?")) return;
			fs::RemoveSystemDataDir();
			try
			{				
				hactool::ExtractHomeMenu();
				hactool::ExtractPlayerSelectMenu();
				hactool::ExtractUserPage();
				Dialog("Done, the home menu was extracted, now you can install nxtheme files !");
			}
			catch (std::runtime_error &err)
			{
				DialogBlocking("Error while extracting the home menu: " + string(err.what()));
			}
		});
	}
	PAGE_RESET_FOCUS;
	
	ImGui::PopFont();
	Utils::ImGuiCloseWin();
}

void NcaDumpPage::Update()
{	
	if (Utils::PageLeaveFocusInput()){
		Parent->PageLeaveFocus(this);
	}
}

void NcaDumpPage::CheckHomeMenuVer()
{
	if (!filesystem::exists(SD_PREFIX "/themes/systemData/ResidentMenu.szs"))
	{
		DialogBlocking("To install custom themes you need to extract the home menu first, this process may take several minutes, don't let your console go to sleep mode and don't press the home button.\nPress A to start");
		goto DUMP_HOMEMENU;
	}
	
	if (filesystem::exists(SD_PREFIX "/themes/systemData/ver.cfg"))
	{
		FILE *ver = fopen(SD_PREFIX "/themes/systemData/ver.cfg", "r");
		if (ver)
		{
			char str[50] = {0};
			fgets(str,49,ver);
			fclose(ver);
			string version(str);
			if (version != SystemVer) goto ASK_DUMP;
			else return;
		}
		else goto ASK_DUMP;
	}
	else if (HOSVer.major >= 7) goto ASK_DUMP;
	else WriteHomeNcaVersion();
	return;
	
ASK_DUMP:
	if (!YesNoPage::Ask("The current firmware version is different than the one of the extracted home menu, do you want to extract the home menu again ?\nIf the extracted home menu doesn't match with the installed one themes will crash."))
	{
		DialogBlocking("You won't see this message again, in case of crashes you can extract the home menu manually from the `Extract home menu` option in the main menu");
		WriteHomeNcaVersion();
		return;
	}
	
DUMP_HOMEMENU:
	fs::RemoveSystemDataDir();
	try
	{
		hactool::ExtractHomeMenu();
	}
	catch (std::runtime_error &err)
	{
		DialogBlocking("Error while extracting the home menu: " + string(err.what()));
	}
}

void NcaDumpPage::WriteHomeNcaVersion()
{
	FILE* ver = fopen(fs::path::NcaVersionCfg.c_str(), "w");
	if (!ver)
		return;
	fprintf(ver, "%s", SystemVer.c_str());
	fclose(ver);
}



