#include "NcaDumpPage.hpp"
#include "../input.hpp"
#include "../ViewFunctions.hpp"
#include "../fs.hpp"
#include "../SwitchTools/hactool.hpp"

using namespace std;

NcaDumpPage::NcaDumpPage() : 
guideText("",WHITE, 870, font25),
 dumpNca("Extract home menu (+)")
{
	Name = "Extract home menu";
	dumpNca.selected = false;
	guideText.SetString("To install .nxtheme files you need to extract the home menu first.\n"
		"To do this you need the switch keys in your sd in a file called prod.keys, "
		"you can get them with lockpick, read the guide at: https://git.io/fxdyF\n\n"
		"Note that for SXOS EMUNAND lockpick doesn't work and you'll have to find them on the internet\n\n"
		"You have to do this EVERY TIME you update (or downgrade) the firmware.\n"
		"Press + to dump the home menu files");
	if (FindKeyFile() == "")
		dumpNca.SetString("Error: Keys not found on the sd card.");
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
		if (FindKeyFile() == "")
		{
			DialogBlocking("Couldn't find the keys on the sd card, place them in one of the following paths:\n"			"sdcard:/keys.prod\nsdcard:/themes/keys.prod\nsdcard:/switch/keys.prod");
			return;
		}
		if ((kHeld & KEY_L) && (kHeld & KEY_R))
		{
			DialogBlocking("Super secret combination entered, only the home menu NCA will be dumped (it won't be extracted)");
			DisplayLoading("Extracting NCA...");
			if (DumpHomeMenuNca())
				Dialog("The home menu NCA was extracted, now use the injector to complete the setup.\nIf you didn't do this on purpose ignore this message.");
			return;
		}
		DialogBlocking("This process may take up to a few minutes, don't let your console go to sleep mode and don't press the home button.\nPress A to start");
		RemoveSystemDataDir();
		if (ExtractHomeMenu())
			Dialog("Done, the home menu was extracted, now you can install nxtheme files !");
	}
	else if (kDown & KEY_B || kDown & KEY_LEFT){
		dumpNca.selected = false;
		Parent->PageLeaveFocus(this);
	}
}




