#include "NcaDumpPage.hpp"
#include "../input.hpp"
#include "../ViewFunctions.hpp"
#include "../fs.hpp"
#include "../SwitchTools/hactool.hpp"
#include <filesystem>

using namespace std;

NcaDumpPage::NcaDumpPage() : 
guideText("",WHITE, 870, font25),
 dumpNca("Extract home menu (+)")
{
	Name = "Extract home menu";
	dumpNca.selected = false;
	guideText.SetString("To install .nxtheme files you need to extract the home menu first.\n"
		"This is done automatically, if you have issues you can try doing it manually here.\n"
		"You have to do this EVERY TIME you update (or downgrade) the firmware.\n"
		"Press + to dump the home menu files");
}

void NcaDumpPage::Render(int X, int Y)
{
	guideText.Render(X + 20, Y + 20);
	dumpNca.Render(X + 20, Y + guideText.GetSize().h + 50);
}

void NcaDumpPage::Update()
{	
	dumpNca.selected = true;
	if (kDown & KEY_PLUS)
	{		
		if ((kHeld & KEY_L) && (kHeld & KEY_R))
		{
			DialogBlocking("Super secret combination entered, only the home menu NCA will be dumped (it won't be extracted)");
			DisplayLoading("Extracting NCA...");
			if (DumpHomeMenuNca())
				Dialog("The home menu NCA was extracted, now use the injector to complete the setup.\nIf you didn't do this on purpose ignore this message.");
			return;
		}
		DialogBlocking("To install custom themes you need to extract the home menu first, this process may take several minutes, don't let your console go to sleep mode and don't press the home button.\nPress A to start");
		RemoveSystemDataDir();
		if (ExtractHomeMenu())
			Dialog("Done, the home menu was extracted, now you can install nxtheme files !");
	}
	else if (kDown & KEY_B || kDown & KEY_LEFT){
		dumpNca.selected = false;
		Parent->PageLeaveFocus(this);
	}
}

extern int NXTheme_FirmMajor;
static void NcaDumpPage::CheckHomeMenuVer()
{
	if (!filesystem::exists("/themes/systemData/ResidentMenu.szs"))
	{
		DialogBlocking("To install custom themes you need to extract the home menu first, this process may take several minutes, don't let your console go to sleep mode and don't press the home button.\nPress A to start");
		goto DUMP_HOMEMENU;
	}
	
	if (filesystem::exists("/themes/systemData/ver.cfg"))
	{
		FILE *ver = fopen("/themes/systemData/ver.cfg", "r");
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



