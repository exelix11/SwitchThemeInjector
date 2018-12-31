#include "UI.hpp"
#include <switch.h>

using namespace std;

Button::Button(const std::string &t, int padding)
{
	Padding = padding;
	Color = {67,67,67,0xff};
	text = new Label(t, WHITE, -1, font30);
	CalcBorder();
}

Button::~Button()
{
	delete text;
}

void Button::CalcBorder()
{
	Border = text->GetSize();
	Border.w += Padding;
	Border.h += Padding;
}

void Button::Render(int X, int Y)
{
	Border.x = X, Border.y = Y;
	if (selected)
	{
		auto b = Border;
		b.x = X - 2; b.y = Y - 2;
		b.w += 4; b.h += 4;
		SDL_SetRenderDrawColor(sdl_render,11,255,209,0xff);
		SDL_RenderDrawRect(sdl_render,&b);
		b.x ++; b.y++;
		b.w -= 2; b.h -= 2;
		SDL_RenderDrawRect(sdl_render,&b);
	}
	if (Highlighted)
		SDL_SetRenderDrawColor(sdl_render,0x36,0x6e,0x64,0xff);
	else
		SDL_SetRenderDrawColor(sdl_render, Color.r,Color.g,Color.b, 0xff);
	SDL_RenderFillRect(sdl_render,&Border);
	auto tSize = text->GetSize();
	text->Render(X + Border.w / 2 - tSize.w / 2, Y + Border.h / 2 - tSize.h / 2);
}

SDL_Rect Button::GetSize() {return Border;}

SDL_Color Button::GetTextColor() {return text->GetColor();}
SDL_Color Button::GetBorderColor() {return Color;}
string Button::GetString() {return text->GetString();}
int Button::GetPadding() {return Padding;}

void Button::SetTextColor(SDL_Color col) {return text->SetColor(col);}
void Button::SetBorderColor(SDL_Color col) {Color = col;}
void Button::SetString(string s) {text->SetString(s); CalcBorder();}
void Button::SetPadding(int p) {Padding = p; CalcBorder();}