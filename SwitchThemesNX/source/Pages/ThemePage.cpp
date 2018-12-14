#include "ThemePage.hpp"
#include "../input.hpp"
#include "../ViewFunctions.hpp"

using namespace std;

ThemesPage::ThemesPage(const std::vector<std::string> &files) : lblPage("",WHITE, -1, font25)
{
	Name = "Themes";
	focused = false;
	ThemeFiles = files;
	pageCount = ThemeFiles.size() / 5 +1;
	if (ThemeFiles.size() % 5 == 0) 
		pageCount--;
	
	if (ThemeFiles.size() == 0)
		NoThemesLbl = new Label("Couldn't find any theme, copy your themes in the themes folder on your sd and try again", WHITE, 870, font25);
	else
		NoThemesLbl = nullptr;
	SetPage(0);	
}

ThemesPage::~ThemesPage()
{
	SetPage(-1);
	if (NoThemesLbl)
		delete NoThemesLbl;
}

void ThemesPage::SetPage(int num)
{
	menuIndex = 0;
	for (auto i : DisplayEntries)
		delete i;
	DisplayEntries.clear();
	
	int baseIndex = num * 5;
	if (num < 0 || baseIndex >= ThemeFiles.size())  
		return;
	
	DisplayLoading("Loading...");
	int imax = ThemeFiles.size() - baseIndex;
	if (imax > 5) imax = 5;
	for (int i = 0; i < imax; i++)
	{
		DisplayEntries.push_back(new ThemeEntry(ThemeFiles[baseIndex + i]));
	}
	pageNum = num;
	lblPage.SetString((string)"Page " + to_string(num + 1) + "/" + to_string(pageCount));
}

const int EntryW = 860;
void ThemesPage::Render(int X, int Y)
{
	if (NoThemesLbl)
	{
		NoThemesLbl->Render(X + 15, Y + 15);
		return;
	}
	
	int RenderY = Y + 20;
	int count = 0;
	for (auto e : DisplayEntries)
	{
		e->Render(X + 16, RenderY, focused && count == menuIndex);		
		RenderY += e->GetRect().h + 15;
		count++;
	}
	lblPage.Render(X + EntryW + 16 - lblPage.GetSize().w, Y + 600);
}

int ThemesPage::PageItemsCount()
{
	int menuCount = ThemeFiles.size() - pageNum * 5;
	if (menuCount > 5)
		menuCount = 5;
	return menuCount;
}

void ThemesPage::Update()
{
	int menuCount = PageItemsCount();	
	if (menuCount <= 0)
		return;
	
	if (kDown & KEY_B || kDown & KEY_LEFT)
		Parent->PageLeaveFocus(this);
	
	if (NoThemesLbl)
		return;
	
	if (kDown & KEY_UP)
	{
		if (menuIndex <= 0)
		{
			if (pageNum > 0)
			{
				SetPage(pageNum - 1);
				menuIndex = PageItemsCount() - 1;
				return;
			}
			else if (pageCount > 1)
			{
				SetPage(pageCount - 1);
				menuIndex = PageItemsCount() - 1;
				return;				
			}
			else menuIndex = menuCount - 1;
		}
		else menuIndex--;
	}
	else if (kDown & KEY_DOWN)
	{
		if (menuIndex >= menuCount - 1)
		{
			if (pageCount > pageNum + 1){
				SetPage(pageNum + 1);
				return;
			}
			else if (pageNum != 0)
			{
				SetPage(0);
				return;
			}
			else menuIndex = 0;
		}
		else menuIndex++;
	}
	else if (kDown & KEY_A)
	{
		if (menuIndex >= 0)
			DisplayEntries[menuIndex]->InstallTheme();
	}
}




