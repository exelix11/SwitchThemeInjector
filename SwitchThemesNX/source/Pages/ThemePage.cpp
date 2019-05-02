#include "ThemePage.hpp"
#include "../input.hpp"
#include "../ViewFunctions.hpp"
#include <algorithm>

using namespace std;

ThemesPage::ThemesPage(const std::vector<std::string> &files) : 
lblPage("",WHITE, -1, font25),
NoThemesLbl("There's nothing here, copy your themes in the themes folder on your sd and try again", WHITE, 870, font25),
lblCommands("", WHITE, -1, font25)
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
	
	pageCount = CurrentFiles.size() / 5 + 1;
	if (CurrentFiles.size() % 5 == 0) 
		pageCount--;
	menuIndex = 0;
	SetPage(0);
}

void ThemesPage::SetPage(int num)
{
	if (pageNum != num)
		menuIndex = 0;
	for (auto i : DisplayEntries)
		delete i;
	DisplayEntries.clear();
	
	lblCommands.SetString(SelectedFiles.size() == 0 ? CommandsTextNormal : CommandsTextSelected);
	
	int baseIndex = num * 5;
	if (num < 0 || baseIndex >= CurrentFiles.size())  
	{
		lblPage.SetString(CurrentDir + " - Empty");
		pageNum = num;
		return;
	}
	
	DisplayLoading("Loading...");
	int imax = CurrentFiles.size() - baseIndex;
	if (imax > 5) imax = 5;
	for (int i = 0; i < imax; i++)
	{
		auto entry = new ThemeEntry(CurrentFiles[baseIndex + i]);
		if (IsSelected(CurrentFiles[baseIndex + i]))
			entry->Highlighted = true;
		DisplayEntries.push_back(entry);
	}
	pageNum = num;
	auto LblPStr = CurrentDir + " - Page " + to_string(num + 1) + "/" + to_string(pageCount);
	if (SelectedFiles.size() != 0)
		LblPStr = "("+ to_string(SelectedFiles.size()) + " selected) " + LblPStr;
	lblPage.SetString(LblPStr);
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
	
	lblCommands.Render(10, Y + 575);
	
	int RenderY = Y + 20;
	int count = 0;
	for (auto e : DisplayEntries)
	{
		e->Render(X + 16, RenderY, focused && count == menuIndex);		
		if ((kHeld & KEY_L) && focused && count == menuIndex && e->HasPreview())
			return;
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

inline bool ThemesPage::IsSelected(const std::string &fname)
{
	return (std::find(SelectedFiles.begin(), SelectedFiles.end(), fname) != SelectedFiles.end());
}

void ThemesPage::SelectCurrent()
{
	if (DisplayEntries[menuIndex]->IsFolder) return;
	auto fname = DisplayEntries[menuIndex]->GetPath();
	auto position = std::find(SelectedFiles.begin(), SelectedFiles.end(), fname);
	if (position != SelectedFiles.end())
	{
		SelectedFiles.erase(position);
	}
	else 
	{
		SelectedFiles.push_back(fname);
	}
	SetPage(pageNum);
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
	else if ((kDown & KEY_A) && menuIndex >= 0 && menuIndex < menuCount)
	{
		if (DisplayEntries[menuIndex]->IsFolder)
			SetDir(DisplayEntries[menuIndex]->GetPath());
		else
		{
			if (SelectedFiles.size() == 0)
			{
				if (kHeld & KEY_R)
				{
					DisplayLoading("Installing to shuffle...");
					DisplayEntries[menuIndex]->InstallTheme(false,MakeThemeShuffleDir());
				}
				else 
					DisplayEntries[menuIndex]->InstallTheme();
			}
			else
				SelectCurrent();
		}
	}
	else if ((kDown & KEY_Y) && menuIndex >= 0)
	{
		SelectCurrent();
	}
	else if ((kDown & KEY_X))
	{
		SelectedFiles.clear();
		SetPage(pageNum);
	}
	else if ((kDown & KEY_PLUS) && SelectedFiles.size() != 0)
	{
		string shuffleDir = "";
		if (kHeld & KEY_R)
			shuffleDir = MakeThemeShuffleDir();
		for (string file : SelectedFiles)
		{
			DisplayLoading("Installing " + file + "...");
			ThemeEntry t {file};
			if (!t.InstallTheme(false,shuffleDir))
			{
				Dialog("Installing a theme failed, the process was cancelled");
				break;
			}
		}
		SelectedFiles.clear();
		SetPage(pageNum);		
	}
}