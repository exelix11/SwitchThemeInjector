#include "UninstallPage.hpp"
#include "../input.hpp"
#include "../ViewFunctions.hpp"

using namespace std;

UninstallPage::UninstallPage() : lblText("Press + to uninstall the currently installed themes",WHITE, -1, font30), btn("Uninstall (+)")
{
	Name = "Uninstall theme";
	btn.selected = false;
	btn.SetBorderColor({107,0,0,0xff});
}

void UninstallPage::Render(int X, int Y)
{
	lblText.Render(X + 20, Y + 20);
	btn.Render(X + 20, Y + 70);
}

void UninstallPage::Update()
{	
	btn.selected = true;
	if (kDown & KEY_PLUS)
	{
		DisplayLoading("Loading...");
		UninstallTheme();
		Dialog("Done, restart your console to see the changes");
	}
	else if (kDown & KEY_B || kDown & KEY_LEFT){
		btn.selected = false;
		Parent->PageLeaveFocus(this);
	}
}




