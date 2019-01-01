#include "ThemeShufflePage.hpp"
#include "../input.hpp"
#include "../ViewFunctions.hpp"

using namespace std;

ShufflePage::ShufflePage() : 
lblGuide(
"Theme shuffle is implemented as a custom sysmodule, get it from: https://git.io/fhtY8 \n"
"To install a theme in the shuffle list press R while pressing A or + in the theme install page",WHITE, 880, font30),
lblSortMode("Theme shuffle mode:",WHITE, 880, font30),
btnRandom("Random"),btnCycle("Cycle"),
lblInstalled("Currently installed themes number: 0",WHITE, 880, font30),
btnClear("Remove all (+)")
{
	Name = "Theme shuffle (BETA)";
	btnClear.SetBorderColor({107,0,0,0xff});
	btnClear.selected = false;
	btnRandom.selected = false;
	btnCycle.selected = false;
	hasFocus = false;
	LoadShuffleState();
}

void ShufflePage::Render(int X, int Y)
{
	int BaseY = Y + 30;
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

void ShufflePage::LoadShuffleState()
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

void ShufflePage::WriteShuffleFlag(int i)
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

void ShufflePage::Update()
{
	if (!hasFocus)
	{
		LoadShuffleState();
		btnClear.selected = false;
		btnRandom.selected = true;
		btnCycle.selected = false;
		selectedIndex = 0;
		hasFocus = true;
		return;
	}
	
	if ((kDown & KEY_LEFT) || (kDown & KEY_B))
	{
		if ((kDown & KEY_B) || selectedIndex == 0)
		{			
			btnClear.selected = false;
			btnRandom.selected = false;
			btnCycle.selected = false;
			hasFocus = false;
			Parent->PageLeaveFocus(this);
			return;			
		}
		else
		{
			btnClear.selected = false;
			btnRandom.selected = true;
			btnCycle.selected = false;
			selectedIndex = 0;
			return;
		}		
	}
	
	if (kDown & KEY_RIGHT)
	{
		btnClear.selected = false;
		btnRandom.selected = false;
		btnCycle.selected = true;
		selectedIndex = 1;
		return;
	}
	
	if (kDown & KEY_UP)
	{
		btnClear.selected = false;
		btnRandom.selected = true;
		btnCycle.selected = false;
		selectedIndex = 0;
		return;
	}
	
	if (kDown & KEY_DOWN)
	{
		btnClear.selected = true;
		btnRandom.selected = false;
		btnCycle.selected = false;
		selectedIndex = 2;
		return;
	}
	
	if (kDown & KEY_A)
	{
		if (selectedIndex == 0)
		{
			WriteShuffleFlag(-1);
		}
		else if (selectedIndex == 1)
		{
			WriteShuffleFlag(0);			
		}
	}
	
	if (selectedIndex == 2 && (kDown & KEY_PLUS))
	{
		ClearThemeShuffle();
		LoadShuffleState();
		Dialog("Theme shuffle deleted");
	}
}









