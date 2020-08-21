#pragma once
#include <iostream>
#include <string>

#include "UI/imgui/imgui.h"
#include "UI/imgui/imgui_internal.h"

#include "UI/DialogPages.hpp"
#include "Platform/Platform.hpp"

const std::string VersionString = "Ver. 2.5";
extern std::string SystemVer;

void PushPage(IUIControlObj* page);
void PopPage(IUIControlObj* page);
void Dialog(const std::string &msg);

//executes a function after the drawing loop is terminated
void PushFunction(const std::function<void()>& fun);

//These functions can only be called during the update function as they need to draw outside of the main loop
void PushPageBlocking(IUIControlObj* page);
void DialogBlocking(const std::string &msg);
void DisplayLoading(const std::string &msg);
void DisplayLoading(std::initializer_list<std::string> lines);

namespace Utils
{
	static inline bool AnyNavButtonPressed()
	{
		return	NAV_UP || NAV_DOWN || NAV_LEFT || NAV_RIGHT;
	}

	static inline void ImGuiNextFullScreen()
	{
		ImGui::SetNextWindowSize({ SCR_W, SCR_H }, ImGuiCond_Always);
		ImGui::SetNextWindowPos({ 0,0 }, ImGuiCond_Always);
	}

	static inline void ImGuiSetupWin(const char* name, int x, int y, ImGuiWindowFlags flags = DefaultWinFlags)
	{
		ImGui::SetNextWindowPos({ (float)x, (float)y}, ImGuiCond_Always);
		ImGui::Begin(name, 0, flags);
	}

	static inline void ImGuiSetupPageFullscreen(const char* name, int x, int y, ImGuiWindowFlags flags = DefaultWinFlags)
	{
		ImGui::SetNextWindowSize({ SCR_W - (float)x - 20, SCR_H - (float)y });
		ImGuiSetupWin(name, x, y, flags);
	}

	static inline void ImGuiSetupPage(const char* name, int x, int y, ImGuiWindowFlags flags = DefaultWinFlags)
	{
		ImGui::SetNextWindowSize(TabPageSize, ImGuiCond_Always);
		ImGuiSetupWin(name, x, y, flags);
	}

	static inline void ImGuiSetupPage(const IPage* page, int x, int y, ImGuiWindowFlags flags = DefaultWinFlags)
	{
		ImGuiSetupPage(page->Name.c_str(), x, y, flags);
	}

	static inline void ImGuiCloseWin()
	{
		ImGui::End();
	}

	void ImGuiDragWithLastElement(); //Defined in UI.cpp

	static inline void ImGuiSetWindowScrollable() 
	{
		if (!AnyNavButtonPressed()) {
			ImGui::SetCursorPos({ 0, 0 });
			ImGui::PushStyleColor(ImGuiCol_Button, { 0,0,0,0 });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0,0,0,0 });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0,0,0,0 });
			ImGui::Button("##drag", ImGui::GetCurrentWindow()->ContentSize - ImVec2(2,2));
			ImGuiDragWithLastElement();
			ImGui::PopStyleColor(3);
		}
	}

	static inline bool ItemNotDragging() 
	{
		return ImGui::GetMouseDragDelta(0).y == 0;
	}

	static inline void ImGuiCenterString(std::string_view str)
	{
		auto win = ImGui::GetWindowWidth();
		auto sz = ImGui::CalcTextSize(str.data(), str.data() + str.size(), false, win);

		ImGui::SetCursorPosX(win/ 2 - sz.x / 2);
		ImGui::TextUnformatted(str.data(), str.data() + str.size());
	}

	template <size_t N, typename T>
	static inline int ImGuiCenterButtons(T (&&buttons)[N])
	{
		const auto str = [&buttons](size_t i) -> const char*
		{
			if constexpr (std::is_same<T, std::string>())
				return buttons[i].c_str();
			else if constexpr (std::is_same<T, const char*>())
				return buttons[i];
		};

		const ImGuiStyle& style = GImGui->Style;
		auto win = ImGui::GetWindowWidth();
		float TotX = 0;
		for (size_t i = 0; i < N; i++)
		{
			auto sz = ImGui::CalcTextSize(str(i), nullptr, false, win);
			TotX += ImGui::CalcItemSize({}, sz.x + style.FramePadding.x * 2.0f, sz.y + style.FramePadding.y * 2.0f).x;
		}
		ImGui::SetCursorPosX((win / 2 - TotX / 2) - (N - 1) * style.FramePadding.x * 2);
		int res = -1;
		for (size_t i = 0; i < N; i++)
		{
			if (ImGui::Button(str(i)))
				res = i;
			if (i != N - 1)
				ImGui::SameLine();
		}
		return res;
	}

	static inline bool ImGuiCenterButton(const std::string& button)
	{
		return ImGuiCenterButtons({ button }) == 0;
	}

	static inline void ImGuiRightString(std::string_view str)
	{
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(str.data(), str.data() + str.size()).x - 2);
		ImGui::TextUnformatted(str.data(), str.data() + str.size());
	}

	static inline bool PageLeaveFocusInput(bool AllowLeft = true)
	{
		return KeyPressed(GLFW_GAMEPAD_BUTTON_B) || (NAV_LEFT && AllowLeft);
	}

	static inline void ImGuiSelectItem(bool isFocused = true, ImGuiID ID = 0)
	{
		auto win = ImGui::GetCurrentWindow();
		if (ID == 0) ID = win->DC.LastItemId;
		ImGui::SetFocusID(ID, win);
		GImGui->NavDisableHighlight = !isFocused;
		GImGui->NavInitResultId = ID;
		ImGui::SetScrollHereY();
	}

#define PAGE_RESET_FOCUS \
	do {if (FocusEvent.Reset() && ImGui::GetFocusID() == 0) Utils::ImGuiSelectItem(true); } while (0)

	static inline bool ImGuiSelectItemOnce(bool isFocused = true)
	{
		if (ImGui::GetCurrentWindow()->Appearing)
		{
			ImGuiSelectItem(isFocused);
			return true;
		}
		return false;
	}
};
