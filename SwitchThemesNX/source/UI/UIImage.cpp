#include "UI.hpp"

using namespace std;

Image::Image(const vector<u8> &data)
{
	_img = LoadImage(data);
}

Image::~Image()
{
	FreeImage(_img);
}

SDL_Rect Image::GetRect() {return _img.Rect;}
void Image::SetRect(SDL_Rect r) {_img.Rect = r;}

void Image::ImageSetSize(int W, int H)
{
	_img.Rect.w = W;
	_img.Rect.h = H;
}

void Image::ImageSetMaxSize(int MaxW, int MaxH)
{
	if (_img.Rect.w > MaxW || _img.Rect.h > MaxH)
	{
		float ratio = (float)_img.Rect.w / _img.Rect.h;
		printf("Original : %d %d %f\n", _img.Rect.w,_img.Rect.h, ratio);
		if (ratio > 1) 
		{
			_img.Rect.w = MaxW;
			_img.Rect.h = MaxW / ratio;
		}
		else 
		{
			_img.Rect.h = MaxH;
			_img.Rect.w = MaxH * ratio;			
		}
		printf("Resized : %d %d\n", _img.Rect.w,_img.Rect.h);
	}
}

void Image::Render(int X,int Y)
{
	if (!Visible)
		return;
	SDL_RenderCopy(sdl_render,_img.image,NULL,&_img.Rect);
}