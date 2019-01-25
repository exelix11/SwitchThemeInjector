#include "TextPage.hpp"
#include "../input.hpp"
#include "../ViewFunctions.hpp"

using namespace std;

TextPage::TextPage(const std::string &title, const std::string &text) : 
Text(text,WHITE, 900, font30)
{
	Name = title;
}

void TextPage::Render(int X, int Y)
{
	Text.Render(X + 20, Y + 20);
}

void TextPage::Update()
{	
	Parent->PageLeaveFocus(this);
}


CreditsPage::CreditsPage() : 
showHelp("Show first startup info"),
creditsText("NXThemes installer by exelix\n" + VersionString + " - Core Ver." + SwitchThemesCommon::CoreVer +
	"\nhttps://github.com/exelix11/SwitchThemeInjector\n\n",WHITE, 900, font30),
creditsText2("Thanks to:\n Syroot for BinaryData lib\n AboodXD for Bntx editor and sarc lib\n XorTroll <3 for his hactool on switch port\n Everyone from Atmosphere and libnx"
,WHITE, 900, font25)
{
	Name = "Credits";
	showHelp.selected = false;
}

void CreditsPage::Render(int X, int Y)
{
	int YRender = 20;
	creditsText.Render(X + 20, Y + YRender);
	YRender += creditsText.GetSize().h + 10;
	creditsText2.Render(X + 20, Y + YRender);
	YRender += creditsText2.GetSize().h + 50;
	showHelp.Render(X + 20, Y + YRender);
}

extern void ShowFirstTimeHelp(bool WelcomeScr); //from main.cpp
void CreditsPage::Update()
{	
	showHelp.selected = true;
	if (kDown & KEY_A)
	{
		ShowFirstTimeHelp(false);
	}
	else if (kDown & KEY_B || kDown & KEY_LEFT){
		showHelp.selected = false;
		Parent->PageLeaveFocus(this);
	}
}

