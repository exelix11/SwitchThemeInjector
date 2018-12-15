#include "ThemePage.hpp"
#include "../input.hpp"
#include "../ViewFunctions.hpp"

using namespace std;

ThemesPage::ThemesPage(const std::vector<std::string> &files) : lblPage("",WHITE, -1, font25),
NoThemesLbl("There's nothing here, copy your themes in the themes folder on your sd and try again", WHITE, 870, font25)
{
	Name = "Themes";
	focused = false;
	ThemeFiles = files;
	
	std::sort(ThemeFiles.begin(), ThemeFiles.end());
	SetDir("/themes");	
}

ThemesPage::~ThemesPage()
{
	SetPage(-1);
}

void ThemesPage::SetDir(const string &dir)
{
	CurrentDir = dir;
	if (!StrEndsWith(dir, "/"))
		CurrentDir += "/";
	
	CurrentFiles.clear();
	for (auto f : ThemeFiles)
	{
		if (GetPath(f) == CurrentDir)
			CurrentFiles.push_back(f);
	}
	
	pageCount = CurrentFiles.size() / 5 +1;
	if (CurrentFiles.size() % 5 == 0) 
		pageCount--;
	SetPage(0);
}

void ThemesPage::SetPage(int num)
{
	menuIndex = 0;
	for (auto i : DisplayEntries)
		delete i;
	DisplayEntries.clear();
	
	int baseIndex = num * 5;
	if (num < 0 || baseIndex >= CurrentFiles.size())  
		return;
	
	DisplayLoading("Loading...");
	int imax = CurrentFiles.size() - baseIndex;
	if (imax > 5) imax = 5;
	for (int i = 0; i < imax; i++)
	{
		DisplayEntries.push_back(new ThemeEntry(CurrentFiles[baseIndex + i]));
	}
	pageNum = num;
	lblPage.SetString(CurrentDir + " - Page " + to_string(num + 1) + "/" + to_string(pageCount));
}

const int EntryW = 860;
void ThemesPage::Render(int X, int Y)
{
	lblPage.Render(X + EntryW + 16 - lblPage.GetSize().w, Y + 600);
	
	if (DisplayEntries.size() == 0)
	{
		NoThemesLbl.Render(X + 15, Y + 15);
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
}

int ThemesPage::PageItemsCount()
{
	int menuCount = CurrentFiles.size() - pageNum * 5;
	if (menuCount > 5)
		menuCount = 5;
	return menuCount;
}

void ThemesPage::Update()
{
	int menuCount = PageItemsCount();	
	
	if (kDown & KEY_LEFT)
		Parent->PageLeaveFocus(this);
	if (kDown & KEY_B)
	{
		if (CurrentDir != "/themes/")
			SetDir(GetParentDir(CurrentDir));
		else 
			Parent->PageLeaveFocus(this);
	}
	
	if (menuCount <= 0)
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
		{
			if (DisplayEntries[menuIndex]->IsFolder)
				SetDir(DisplayEntries[menuIndex]->GetPath());
			else
				DisplayEntries[menuIndex]->InstallTheme();
		}
	}
}




