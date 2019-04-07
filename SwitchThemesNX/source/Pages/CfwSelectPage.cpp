#include "CfwSelectPage.hpp"
#include "../ViewFunctions.hpp"
#include "../input.hpp"

using namespace std;

CfwSelectPage::CfwSelectPage(vector<string> &folders) : Title("",WHITE, 1000, font30)
{
	Folders = folders;
	if (folders.size() == 0)
		Title.SetString("Couldn't find any cfw folder. Make sure you have either the \"atmosphere\", \"reinx\" or \"sxos\" folder in the root of your sd card as some cfws don't create it automatically. The folder name must be lowercase and without spaces.\nif your cfw isn't supported open an issue on Github.\nPress home to quit");
	else 
		Title.SetString("Multiple cfw folders were detected, which one do you want to use ?");
	pageCount = folders.size() / 5 +1;
	SetPage(0);	
}

CfwSelectPage::~CfwSelectPage()
{
	SetPage(-1);
}

void CfwSelectPage::SetPage(int num)
{
	menuIndex = 0;
	for (auto i : DisplayEntries)
		delete i;
	DisplayEntries.clear();
	
	int baseIndex = num * 5;
	if (num < 0 || baseIndex >= Folders.size())  
		return;
	
	int imax = Folders.size() - baseIndex;
	if (imax > 5) imax = 5;
	for (int i = 0; i < imax; i++)
	{
		DisplayEntries.push_back(new Label(Folders[baseIndex + i], WHITE, -1, font30));
	}
	pageNum = num;
}

void CfwSelectPage::Render(int X, int Y)
{	
	SDL_SetRenderDrawColor(sdl_render,45,45,45,0xff); //Switch dark bg
	SDL_RenderFillRect(sdl_render,&ScreenRect);

	Title.Render(SCR_W / 2 - Title.GetSize().w / 2,70);
	int RenderY = 70 + Title.GetSize().h + 30;
	int count = 0;
	for (auto e : DisplayEntries)
	{
		auto size = e->GetSize();
		size.x = SCR_W /2 - size.w/2;
		size.y = RenderY;
		if (count == menuIndex)
		{
			size.x -= 4;
			size.y -= 4;
			size.w += 8;
			size.h += 8;
			SDL_SetRenderDrawColor(sdl_render,11,255,209,0xff);
			SDL_RenderFillRect(sdl_render,&size);
			size.x += 2;
			size.y += 2;
			size.w -= 4;
			size.h -= 4;			
			SDL_SetRenderDrawColor(sdl_render,45,45,45,0xff);
			SDL_RenderFillRect(sdl_render,&size);
			size.x += 2;
			size.y += 2;
			size.w -= 4;
			size.h -= 4;
		}
		e->Render(size.x, RenderY);		
		RenderY += size.h + 15;
		count++;
	}
}

int CfwSelectPage::PageItemsCount()
{
	int menuCount = Folders.size() - pageNum * 5;
	if (menuCount > 5)
		menuCount = 5;
	else if (menuCount < 0)
		menuCount = 0;
	return menuCount;
}

void CfwSelectPage::Update()
{
	int menuCount = PageItemsCount();	
	if (menuCount <= 0)
		return;
	
	if (!kDown)
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
		CfwFolder = DisplayEntries[menuIndex]->GetString();
		PopPage();
	}
}




