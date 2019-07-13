#include "NcaDumpPage.hpp"
#include "../ViewFunctions.hpp"
#include "../fs.hpp"
#include "../SwitchTools/hactool.hpp"
#include <filesystem>
#include "../Platform/Platform.hpp"

using namespace std;

NcaDumpPage::NcaDumpPage() : 
 dumpNca("Extract home menu")
{
	Name = "Extract home menu";
	guideText = ("To install .nxtheme files you need to extract the home menu first.\n"
		"This is done automatically, if you have issues you can try doing it manually here.\n"
		"You have to do this EVERY TIME you update (or downgrade) the firmware.\n"
		"Press + to dump the home menu files");
}

void NcaDumpPage::Render(int X, int Y)
{
	Utils::ImGuiSetupPage(this, X, Y);
	ImGui::PushFont(font30);

	ImGui::TextWrapped(guideText.c_str());
	if (ImGui::Button(dumpNca.c_str()))
	{
		PushFunction([]() {
			if ((gamepad.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] && gamepad.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER]))
			{
				DialogBlocking("Super secret combination entered, only the home menu NCA will be dumped (it won't be extracted)");
				DisplayLoading("Extracting NCA...");
				if (DumpHomeMenuNca())
					Dialog("The home menu NCA was extracted, now use the injector to complete the setup.\nIf you didn't do this on purpose ignore this message.");
				return;
			}
			if (!YesNoPage::Ask(
				"To install custom themes you need to extract the home menu first, this process may take several minutes, don't let your console go to sleep mode and don't press the home button.\n"
				"Do you want to continue ?")) return;
			RemoveSystemDataDir();
			if (ExtractHomeMenu())
				Dialog("Done, the home menu was extracted, now you can install nxtheme files !");
		});
	}
	PAGE_RESET_FOCUS
	
	ImGui::PopFont();
	Utils::ImGuiCloseWin();
}

void NcaDumpPage::Update()
{	
	if (Utils::PageLeaveFocusInput()){
		Parent->PageLeaveFocus(this);
	}
}

extern int NXTheme_FirmMajor;
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
			char str[50];
			fgets(str,50,ver);
			fclose(ver);
			string version(str);
			if (version != SystemVer) goto ASK_DUMP;
			else return;
		}
		else goto ASK_DUMP;
	}
	else if (NXTheme_FirmMajor >= 7) goto ASK_DUMP;
	else WriteHomeDumpVer();
	return;
	
ASK_DUMP:
	if (!YesNoPage::Ask("The current firmware version is different than the one of the extracted home menu, do you want to dump the home menu again ?\nIf the extracted home menu doesn't match with the installed one themes will crash."))
		return;
	
DUMP_HOMEMENU:
	RemoveSystemDataDir();
	ExtractHomeMenu();
}



