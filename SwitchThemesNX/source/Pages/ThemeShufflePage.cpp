#include "ThemeShufflePage.hpp"
#include "../input.hpp"
#include "../ViewFunctions.hpp"

using namespace std;

SettingsPage::SettingsPage() : 
lblAnimSupport("",WHITE, -1, font25),
btnAnimEnable(""),
lblShuffleSettings("Theme shuffle settings (BETA) : ",WHITE, -1, font25),
lblGuide(
"Theme shuffle is implemented as a custom sysmodule, get it from: https://git.io/fhtY8 \n"
"To install a theme in the shuffle list press R while pressing A or + in the theme install page",WHITE, 880, font30),
lblSortMode("Theme shuffle mode:",WHITE, 880, font30),
btnRandom("Random"),btnCycle("Cycle"),
lblInstalled("Currently installed themes number: 0",WHITE, 880, font30),
btnClear("Remove all (+)")
{
	Name = "Settings";
	btnClear.SetBorderColor({107,0,0,0xff});
	btnClear.selected = false;
	btnRandom.selected = false;
	btnCycle.selected = false;
	btnAnimEnable.selected = false;
	
	UpdateAnimState();
	
	hasFocus = false;
	LoadShuffleState();
}

void SettingsPage::UpdateAnimState()
{	
	btnAnimEnable.SetString(UseAnimations ? "Disable" : "Enable");
	lblAnimSupport.SetString(UseAnimations ? "Animation support is enabled" : "Animation support is disabled");
}

void SettingsPage::Render(int X, int Y)
{
	int BaseY = Y + 30;
	lblAnimSupport.Render(X + 20, BaseY);
	auto sz = lblAnimSupport.GetSize();
	btnAnimEnable.Render(X + 30 + sz.x, BaseY);
	BaseY += sz.y + 20;
	lblShuffleSettings.Render(X + 20, BaseY);
	BaseY += lblShuffleSettings.GetSize().h + 20;	
	lblGuide.Render(X + 20, BaseY);
	BaseY += lblGuide.GetSize().h + 20;
	lblSortMode.Render(X + 20, BaseY);
	BaseY += lblSortMode.GetSize().h + 20;
	btnRandom.Render(X + 20, BaseY);
	auto BtnSz = btnRandom.GetSize();
	btnCycle.Render(X + 40 + BtnSz.w , BaseY);
	BaseY += BtnSz.h + 20;
	lblInstalled.Render(X + 20, BaseY);
	BaseY += lblInstalled.GetSize().h + 20;
	btnClear.Render(X + 20, BaseY);
}

void SettingsPage::LoadShuffleState()
{
	lblInstalled.SetString("Currently installed themes number: " + to_string(GetShuffleCount()));
	FILE *index = fopen("sdmc:/themes/shuffle/index.db", "r");
	if (index)
	{
		int t = 0;
		fscanf(index,"%d",&t);
		if (t < 0)
			btnRandom.Highlighted = true;
		else 
			btnCycle.Highlighted = true;	
		fclose(index);
	}
}

void SettingsPage::WriteShuffleFlag(int i)
{
	FILE *index = fopen("sdmc:/themes/shuffle/index.db", "w");
	if (index)
	{
		btnRandom.Highlighted = false;
		btnCycle.Highlighted = false;
		fprintf(index,"%d",i);
		if (i < 0)
			btnRandom.Highlighted = true;
		else 
			btnCycle.Highlighted = true;	
		fclose(index);
	}
	else
		Dialog("Error: couldn't write to file. Maybe you don't have any theme in the shuffle.");
}

void SettingsPage::ClearBtnState()
{
	btnClear.selected = false;
	btnRandom.selected = false;
	btnCycle.selected = false;
	btnAnimEnable.selected = false;
}

/*
Page layout:
~At times like this i regret not using imgui~

btnAnimEnable index = 0
v^
btnRandom (index = 1) <> btnCycle (index = 2)
v^
btnClear (index = 3)
*/

void SettingsPage::Update()
{
	if (!hasFocus)
	{
		LoadShuffleState();
		ClearBtnState();
		btnAnimEnable.selected = true;
		selectedIndex = 0;
		hasFocus = true;
		return;
	}
	
	if ((kDown & KEY_LEFT) || (kDown & KEY_B))
	{
		if ((kDown & KEY_B) || selectedIndex == 0)
		{
			ClearBtnState();
			hasFocus = false;
			Parent->PageLeaveFocus(this);
			return;			
		}
		else if (selectedIndex == 2)
		{
			ClearBtnState();
			btnRandom.selected = true;
			selectedIndex = 1;
			return;
		}		
	}
	
	if (kDown & KEY_RIGHT)
	{
		ClearBtnState();
		btnCycle.selected = true;
		selectedIndex = 2;
		return;
	}
	
	if (kDown & KEY_UP)
	{
		ClearBtnState();
		if (selectedIndex == 1 || selectedIndex == 2) 
		{			
			btnAnimEnable.selected = true;
			selectedIndex == 0;
		}
		else if (selectedIndex == 3)
		{
			btnRandom.selected = true;
			selectedIndex = 1;
		}
		return;
	}
	
	if (kDown & KEY_DOWN)
	{
		ClearBtnState();
		if (selectedIndex == 0) 
		{			
			btnRandom.selected = true;
			selectedIndex == 1;
		}
		else if (selectedIndex == 1 || selectedIndex == 2)
		{
			btnClear.selected = true;
			selectedIndex = 3;
		}
		return;
	}
	
	if (kDown & KEY_A)
	{
		if (selectedIndex == 0)
		{
			UseAnimations = !UseAnimations;
			UpdateAnimState();
		}
		else if (selectedIndex == 1)
		{
			WriteShuffleFlag(-1);
		}
		else if (selectedIndex == 2)
		{
			WriteShuffleFlag(0);			
		}
	}
	
	if (selectedIndex == 3 && (kDown & KEY_PLUS))
	{
		ClearThemeShuffle();
		LoadShuffleState();
		Dialog("Theme shuffle deleted");
	}
}









