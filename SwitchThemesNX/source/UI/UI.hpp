#pragma once
#include "../SwitchThemesCommon/MyTypes.h"
#include <iostream>
#include <vector>
#include <string>
#include <switch.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#define SCR_W 1280
#define SCR_H 720

const SDL_Color WHITE = {0xff,0xff,0xff};
const SDL_Color BLACK = {0,0,0};

extern SDL_Window* sdl_win;
extern SDL_Renderer* sdl_render;

void SdlInit();
void SdlExit();

extern TTF_Font *font20;
extern TTF_Font *font25;
extern TTF_Font *font30;
extern TTF_Font *font40;

void FontInit();
void FontExit();

extern const SDL_Rect ScreenRect;

struct LoadedImage
{
	SDL_Rect Rect;
	SDL_Texture* image;
};

LoadedImage OpenImage(const std::string &Path);
LoadedImage LoadImage(const std::vector<u8> &data);
void FreeImage(LoadedImage &img);

LoadedImage LoadImage(std::string URL);

#define defProp(name,type) type Get ## name(); void Set ## name(type arg); 

class Label
{
	private:
		void RenderString();
		SDL_Texture* tex = NULL;
		std::string string = "";
		int wrap = -1;
		SDL_Color color = WHITE;
		SDL_Rect rect = {0,0,0,0};
		TTF_Font* font;
	public:
		SDL_Rect GetSize();
		
		defProp(String,std::string)
		defProp(Wrap,int)
		defProp(Color,SDL_Color)
		defProp(Font,TTF_Font*)
	
		Label(const std::string &str, SDL_Color _color, int _wrap, TTF_Font* fnt = font20);
		~Label();
		void Render(int X, int Y);
};

class Button
{
	private:
		Label *text;
		SDL_Rect Border;
		SDL_Color Color;
		int Padding;
		void CalcBorder();
	public:
		Button(const std::string &text, int padding = 15);
		~Button();
		bool selected;
		bool Highlighted = false;
		
		void Render(int X, int Y);
		
		SDL_Rect GetSize();
		defProp(Padding,int)
		defProp(String,std::string)
		defProp(TextColor,SDL_Color)
		defProp(BorderColor,SDL_Color)
};

class IUIControlObj
{
	public:
		virtual void Update() = 0;
		virtual void Render(int X, int Y) = 0;
		virtual ~IUIControlObj();
	};

class TabRenderer;
class IPage : public IUIControlObj
{
	public:
		bool focused;
		TabRenderer* Parent;
		std::string Name;
		virtual ~IPage();
};

class TabRenderer : public IUIControlObj
{
	public:
		TabRenderer();
	
		//TabRenderer ignores the position
		void Render(int X, int Y) override;

		void PageLeaveFocus(IPage *page);
		void AddPage(IPage* page);
		void RemoveAt(int id);
		IPage* At(int id);
		
		void Update() override;
	private:
		IPage* FocusedControl = nullptr;
		int selectedPage = 0;
		std::vector<IPage*> Pages;
		std::vector<Label*> PageLables;
		
		Label Title;
};

class Image
{
	public: 
		bool Visible = true;
	
		Image(const std::vector<u8> &data);
		~Image();
		
		defProp(Rect,SDL_Rect);
		
		void ImageSetMaxSize(int MaxW, int MaxH);
		void ImageSetSize(int W, int H);
		void Render(int X, int Y);
	private:
		LoadedImage _img;
};