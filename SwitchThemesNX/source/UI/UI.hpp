#pragma once
#include "../SwitchThemesCommon/MyTypes.h"
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include "imgui\imgui.h"

#ifdef  __SWITCH__
#include <switch.h>
#endif

extern ImFont* font25;
extern ImFont* font30;
extern ImFont* font40;

constexpr uint32_t SCR_W = 1280;
constexpr uint32_t SCR_H = 720;

const ImVec2 TabPageArea = { 900, 552 };

typedef intptr_t LoadedImage;

namespace ImageCache {
	void FreeImage(const std::string& img);
	LoadedImage LoadDDS(const std::vector<u8>& data, const std::string& name);
};

struct PageEvent
{
	bool Reset() { if (Fired) { Fired = false; return true; } return false; }
	void Set() { Fired = true; }
	bool Peek() { return Fired; }
private:
	bool Fired = true;
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

		PageEvent FocusEvent;
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
		void SetFocused(int id);
		IPage* CurrentControl = nullptr;
		bool ControlHasFocus = false;
		std::vector<IPage*> Pages;
		std::string Title;
};