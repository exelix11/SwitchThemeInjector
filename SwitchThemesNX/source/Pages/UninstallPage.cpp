#include "UninstallPage.hpp"
#include "../input.hpp"
#include "../ViewFunctions.hpp"

using namespace std;

UninstallPage::UninstallPage() : 
lblText("Press + to uninstall the currently installed themes.\nIf you have issues, try fully removing the LayeredFS directory by pressing L+R as well.",WHITE, 870, font30), btn("Uninstall (+)")
{
	Name = "Uninstall theme";
	btn.selected = false;
	btn.SetBorderColor({107,0,0,0xff});
}

void UninstallPage::Render(int X, int Y)
{
	lblText.Render(X + 20, Y + 20);
	btn.Render(X + 20, Y + 30 + lblText.GetSize().h);
}

void UninstallPage::Update()
{	
	btn.selected = true;
	if (kDown & KEY_PLUS)
	{
		if ((kHeld & KEY_L) && (kHeld & KEY_R))
		{			
			DisplayLoading("Clearing LayeredFS dir...");
			UninstallTheme(true);
			Dialog("Done, the layeredFS dir of the home menu was removed, restart your console to see the changes");
		}
		else
		{
			DisplayLoading("Loading...");
			UninstallTheme(false);
			Dialog("Done, all the installed themes have been removed, restart your console to see the changes");
		}
	}
	else if (kDown & KEY_B || kDown & KEY_LEFT){
		btn.selected = false;
		Parent->PageLeaveFocus(this);
	}
}




