#include "UI.hpp"
#include <algorithm> 
using namespace std;

Label::Label(const std::string &str, SDL_Color _color, int _wrap,TTF_Font* fnt):string(str), color(_color), wrap(_wrap), font(fnt)
{
	RenderString();
}

Label::~Label()
{
	if (tex != NULL)
		SDL_DestroyTexture(tex);
	tex = NULL;	
}

void Label::RenderString()
{	
	if (string == "" || string == " ")
		string = ".";
	
	if (tex)
	{
		SDL_DestroyTexture(tex);
		tex = nullptr;
	}
	
	SDL_Surface* surf = nullptr;
	if (wrap == -1)
		surf = TTF_RenderText_Blended(font, string.c_str(),color);
	else 
		surf = TTF_RenderText_Blended_Wrapped(font, string.c_str(),color,wrap);
	if (!surf)
	{		
		SetString("<font draw error:" + std::string(TTF_GetError()) + ">");
		return;
	}	
	tex = SDL_CreateTextureFromSurface(sdl_render, surf);
	SDL_Rect textLocation;
	textLocation.x = 0;
	textLocation.y = 0;
	textLocation.w = surf->w;
	textLocation.h = surf->h;
	rect = textLocation;
	SDL_FreeSurface(surf);	
}

void Label::SetString(std::string str)
{
	string = str;
	RenderString();	
}

std::string Label::GetString() {return string;}

void Label::SetFont(TTF_Font* fnt)
{
	font = fnt;
	RenderString();	
}

TTF_Font* Label::GetFont() {return font;}

void Label::SetWrap(int w)
{
	wrap = w;
	RenderString();	
}

int Label::GetWrap() {return wrap;}

void Label::SetColor(SDL_Color c)
{
	color = c;
	RenderString();	
}

SDL_Color Label::GetColor() {return color;}

SDL_Rect Label::GetSize() {return rect;}

void Label::Render(int X, int Y)
{
	rect.x = X;
	rect.y = Y;
	SDL_RenderCopy(sdl_render,tex,NULL,&rect);
}