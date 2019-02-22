#include "DialogPages.hpp"
#include "../input.hpp"
#include "../ViewFunctions.hpp"

using namespace std;

LoadingOverlay::LoadingOverlay(const string &msg) : text(msg,100) {}

void LoadingOverlay::Render(int X, int Y)
{		
	auto s = text.GetSize();
	text.Render(SCR_W / 2 - s.w / 2, SCR_H/2 - s.h/2);
}

void LoadingOverlay::Update() {}

FatalErrorPage::FatalErrorPage(const string &msg) : text(msg,WHITE, 1000, font30) {}

void FatalErrorPage::Render(int X, int Y)
{	
	SDL_SetRenderDrawColor(sdl_render,45,45,45,0xff); //Switch dark bg
	SDL_RenderFillRect(sdl_render,&ScreenRect);

	text.Render(SCR_W / 2 - text.GetSize().w / 2,70);
}

void FatalErrorPage::Update(){}

DialogPage::DialogPage(const string &msg,const string &buttonMsg) : text(msg,WHITE, 1000, font30), Btn(buttonMsg, WHITE, -1, font30){}
DialogPage::DialogPage(const string &msg) : DialogPage(msg, "Continue") {}

void DialogPage::Render(int X, int Y)
{	
	SDL_SetRenderDrawColor(sdl_render,45,45,45,0xff);
	SDL_RenderFillRect(sdl_render,&ScreenRect);

	text.Render(SCR_W / 2 - text.GetSize().w / 2,70);
	
	auto size = Btn.GetSize();
	size.x = SCR_W/2 - size.w/2; size.y = SCR_H - 50 - size.h;
	size.w += 24; size.h += 24,
	size.x -= 12; size.y -= 12;
	SDL_SetRenderDrawColor(sdl_render,11,255,209,0xff); 
	SDL_RenderFillRect(sdl_render,&size);
	size.w -= 4; size.h -= 4,
	size.x += 2; size.y += 2;
	SDL_SetRenderDrawColor(sdl_render,60,60,60,0xff); 
	SDL_RenderFillRect(sdl_render,&size);
	
	size = Btn.GetSize();
	Btn.Render(SCR_W/2 - size.w/2, SCR_H - 50 - size.h);
}

void DialogPage::Update()
{
	if (kDown & KEY_A)
		PopPage();
}

YesNoPage::YesNoPage(const string &msg, bool *outRes): text(msg, WHITE, 1000, font30), btnYes("   Yes   "), btnNo("   No   ") 
{
	result = outRes;
	btnYes.selected = true;
	btnNo.selected = false;
}

void YesNoPage::Render(int X, int Y)
{
	SDL_SetRenderDrawColor(sdl_render,45,45,45,0xff);
	SDL_RenderFillRect(sdl_render,&ScreenRect);
	
	auto TextSize = text.GetSize();

	text.Render(SCR_W / 2 - TextSize.w / 2,70);
	
	auto YesSize = btnYes.GetSize();
	auto NoSize = btnNo.GetSize();
	
	int btnX =  SCR_W/2 - (YesSize.w + 30 + NoSize.w)/2;
	
	btnYes.Render(btnX, TextSize.h + 90);
	btnNo.Render(btnX + YesSize.w + 30, TextSize.h + 90);
}

void YesNoPage::Update()
{
	if (kDown & KEY_A)
	{
		*result = btnYes.selected;
		PopPage();
		return;
	}
	else if (kDown & KEY_RIGHT)
	{
		btnYes.selected = false;
		btnNo.selected = true;
	}
	else if (kDown & KEY_LEFT)
	{
		btnYes.selected = true;
		btnNo.selected = false;
	}
}

bool YesNoPage::Ask(const std::string &msg)
{
	bool result = false;
	PushPageBlocking(new YesNoPage(msg, &result));
	return result;
}

