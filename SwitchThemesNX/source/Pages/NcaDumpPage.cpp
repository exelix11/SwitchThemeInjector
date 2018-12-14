#include "NcaDumpPage.hpp"
#include "../input.hpp"
#include "../ViewFunctions.hpp"

using namespace std;

NcaDumpPage::NcaDumpPage() : 
guideText(
"To install .nxtheme files you need the base files of your home menu in your sd card,"
"the extraction process now has been made simpler: no more mounting nand or dangerous "
"operations, you just need to run a few homebrews and a PC.\n"
"read the new dumping guide at: \n\n"
"Press + to dump the home menu NCA"
,WHITE, 900, font30),
 dumpNca("Dump NCA (+)")
{
	Name = "Dump NCA";
	dumpNca.selected = false;
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
		DisplayLoading("Loading...");
		if (DumpHomeMenuNca())
			Dialog("Done, the nca was saved in sd:/themes/systemData/home.nca");
	}
	else if (kDown & KEY_B || kDown & KEY_LEFT){
		dumpNca.selected = false;
		Parent->PageLeaveFocus(this);
	}
}




