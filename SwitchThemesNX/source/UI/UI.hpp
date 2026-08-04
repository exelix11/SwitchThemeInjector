#pragma once
#include "../SwitchThemesCommon/MyTypes.h"
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include "imgui/imgui.h"

#ifdef  __SWITCH__
#include <switch.h>
#endif
#include <memory>

constexpr ImGuiWindowFlags DefaultWinFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove;

const ImVec2 TabPageSize = { 900, 552 };

extern ImFont* font25;
extern ImFont* font30;
extern ImFont* font40;

constexpr uint32_t SCR_W = 1280;
constexpr uint32_t SCR_H = 720;

namespace Colors 
{
	const ImVec4 Highlight = { 0,1,0.788f,1 };
	const ImVec4 Red = { 1,.2f,.2f,1 };
}

class RenderImage 
{
public:
	int Width;
	int Height;
	ImTextureID TextureId;

	bool IsValid() const { return TextureId != 0 && Width != 0 && Height != 0; }

	RenderImage(const std::vector<u8>& data);
	RenderImage() : Width(0), Height(0), TextureId(0) {}

	RenderImage(const RenderImage&) = delete;
	RenderImage& operator=(const RenderImage&) = delete;

	RenderImage(RenderImage&&);
	RenderImage& operator=(RenderImage&&);

	~RenderImage();

	void Release();

	static void DebugAssertLeaks();
	static size_t DebugLoadedImages();
private:
	static size_t LeakCount;
	void Invalidate() { TextureId = 0; Width = 0; Height = 0; }
};

using ImageRef = std::shared_ptr<RenderImage>;

namespace ImageCache 
{
	void Clear();
	void FreeImage(const std::string& img);

	//Cache automatically frees old images, no need to do it manually
	ImageRef Load(const std::vector<u8>& data, const std::string& name);
	ImageRef Get(const std::string& name);
	void PopOne();

	void DebugInformation(size_t& out_size, int& out_count, float& out_percent);
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
		//Execute commands out of the drawing code,eg manually check inputs or call UI blocking functions
		virtual void Update() = 0;
		//Draw the control in the existing Imgui frame
		virtual void Render(int X, int Y) = 0;
		virtual ~IUIControlObj() = default;
};

class TabRenderer;
class IPage : public IUIControlObj
{
	public:
		IPage() = default;
		IPage(std::string_view name) : Name(name) {}
		virtual ~IPage() = default;
		
		TabRenderer* Parent = nullptr;
		std::string Name = "";
		PageEvent FocusEvent = {};
		bool NotificationIcon = false;
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