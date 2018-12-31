#include "NcaDumpPage.hpp"
#include "../input.hpp"
#include "../ViewFunctions.hpp"
#include "../fs.hpp"
#include "../hactool/hactool.hpp"

using namespace std;

NcaDumpPage::NcaDumpPage() : 
guideText("",WHITE, 880, font30),
 dumpNca("Extract home menu (+)")
{
	Name = "Extract home menu";
	dumpNca.selected = false;
	guideText.SetString("To install .nxtheme files you need to extract the base files of your home menu in your sd card,"
		"to extract the files now you just need the switch keys on the sd,"
		"you can get them with lockpick, read the guide at: https://git.io/fhLim\n"
		"Note that for SXOS EMUNAND users lockpick doesn't work and you'll have to find them on the internet\n"
		"Remember that you have to do this EVERY TIME you update (or downgrade) the firmware.\n"
		"Press + to dump the home menu files");
	if (FindKeyFile() == "")
		dumpNca.SetString("Keys not found: extract just NCA (+)");
}

void NcaDumpPage::Render(int X, int Y)
{
	guideText.Render(X + 20, Y + 20);
	dumpNca.Render(X + 20, Y + guideText.GetSize().h +70);
}

void NcaDumpPage::Update()
{	
	dumpNca.selected = true;
	if (kDown & KEY_PLUS)
	{
		if (FindKeyFile() == "")
		{
			Dialog("Couldn't find the keys on the sd card, place them in one of the following paths:\n"			"sdcard:/keys.prod\nsdcard:/themes/keys.prod\nsdcard:/switch/keys.prod");
			Dialog("Only the NCA of the home menu will be dumped, you'll have to extract the files from the injector following this guide: https://git.io/fxdyF \n"
			"(Or put the keys on the sd card and repeat this process.)");
			RemoveSystemDataDir();
			if (DumpHomeMenuNca())
				Dialog("Done, the home menu NCA was extracted, now use the injector to complete the setup.");
			return;
		}
		Dialog("This process may take up to a few minutes, don't let your console go to sleep mode and don't press the home button.\nPress A to start");
		RemoveSystemDataDir();
		if (ExtractHomeMenu())
			Dialog("Done, the home menu was extracted, now you can install nxtheme files !");
	}
	else if (kDown & KEY_B || kDown & KEY_LEFT){
		dumpNca.selected = false;
		Parent->PageLeaveFocus(this);
	}
}




