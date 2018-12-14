#include "UI.hpp"
#include "../ViewFunctions.hpp"
#include "../input.hpp"
using namespace std;

const SDL_Rect ScreenRect = {0,0,SCR_W,SCR_H};
const SDL_Rect BottomRect = {0, SCR_H - 67, SCR_W, 67};
const SDL_Rect TopRect = {0,0,SCR_W,76};
const SDL_Rect SideRect = {0,TopRect.h,378,BottomRect.y - TopRect.h};

#define TopLineLen 1200
#define SideLineLen 510

const SDL_Rect TopLine = {SCR_W/2 - TopLineLen/2, TopRect.h + 1,TopLineLen,2};
const SDL_Rect BottomLine = {SCR_W/2 - TopLineLen/2, BottomRect.y - 1,TopLineLen,2};
const SDL_Rect SideLine = {SideRect.w + 1, (SCR_H)/2 - SideLineLen /2 ,2,SideLineLen};

void TabRenderer::Render(int X, int Y)
{
	SDL_SetRenderDrawColor(sdl_render,45,45,45,0xff); //Switch dark bg
	SDL_RenderFillRect(sdl_render,&ScreenRect);
	
	//SDL_SetRenderDrawColor(sdl_render,45,45,45,0xff);
	//SDL_RenderFillRect(sdl_render,&SideRect);
	
	int BaseLabelY = TopRect.h + 15;
	int count = 0;
	for (auto label : PageLables)
	{
		auto lSize = label->GetSize();
		if (count == selectedPage){
			auto border = lSize;
			border.x = TopLine.x; border.y = BaseLabelY - 4; 
			border.w = SideRect.w - TopLine.x - 4; border.h += 8;
			if (!FocusedControl)
			{
				SDL_SetRenderDrawColor(sdl_render,11,255,209,0xff);
				SDL_RenderDrawRect(sdl_render,&border); //two pixels border
				border.x++; border.y++;
				border.w -= 2; border.h -=2;
				SDL_RenderDrawRect(sdl_render,&border);
			}
			else 
			{
				SDL_SetRenderDrawColor(sdl_render,90,90,90,0xff);
				SDL_RenderFillRect(sdl_render,&border);			
			}
		}
		label->Render(TopLine.x + 4,BaseLabelY);
		BaseLabelY += label->GetSize().h + 4;
		++count;
	}
	
	SDL_SetRenderDrawColor(sdl_render,45,45,45,0xff);
	SDL_RenderFillRect(sdl_render,&BottomRect);
	SDL_RenderFillRect(sdl_render,&TopRect);
	SDL_SetRenderDrawColor(sdl_render,0xff,0xff,0xff,0xff);
	SDL_RenderFillRect(sdl_render,&TopLine);
	SDL_RenderFillRect(sdl_render,&BottomLine);
	SDL_RenderFillRect(sdl_render,&SideLine);
	
	Title.Render(21,21);
	
	if (selectedPage >= 0)
		Pages[selectedPage]->Render(SideRect.w + 5, TopRect.h + 5);	
}

TabRenderer::TabRenderer() :
Title("NXThemes Installer " + VersionString, WHITE, -1, font30)
{
	FocusedControl = nullptr;
}

void TabRenderer::AddPage(IPage* page) 
{
	page->Parent = this;
	Pages.push_back(page);
	Label *lbl = new Label(page->Name,WHITE,-1,font30);
	PageLables.push_back(lbl);
}

void TabRenderer::RemoveAt(int id)
{
	Pages.erase(Pages.begin() + id);
	delete PageLables[id];
	PageLables.erase(PageLables.begin() + id);
}

IPage* TabRenderer::At(int id)
{
	return Pages[id];
}

void TabRenderer::PageLeaveFocus(IPage *page)
{
	FocusedControl->focused = false;
	FocusedControl = nullptr;
}

void TabRenderer::Update()
{
	if (FocusedControl) 
	{
		FocusedControl->Update();
		return;
	}
	if (Pages.size() == 0)
		return;
	if (kDown & KEY_UP)
		selectedPage = selectedPage == 0 ? Pages.size() - 1: selectedPage - 1;
	else if (kDown & KEY_DOWN)
		selectedPage = selectedPage == Pages.size() - 1 ? 0 : selectedPage + 1;
	else if (kDown & KEY_A || kDown & KEY_RIGHT)
	{
		FocusedControl = Pages[selectedPage];
		FocusedControl->focused = true;
	}
}


