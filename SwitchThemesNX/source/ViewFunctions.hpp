#pragma once
#include <iostream>
#include <string>
#include "UI/DialogPages.hpp"
#include "Platform/Platform.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "UI/imgui/imgui_internal.h"

const std::string VersionString = "Ver. 2.0";
extern std::string SystemVer;

extern bool UseAnimations;

void PushPage(IUIControlObj* page);
void PopPage();
void ErrorFatal(const std::string &msg);
void Dialog(const std::string &msg);

//These functions can only be called during the update function as they need to draw outside of the main loop
//executes a function after the drawing loop is terminated
void PushFunction(const std::function<void()>& fun);
void PushPageBlocking(IUIControlObj* page);
void DialogBlocking(const std::string &msg);
void DisplayLoading(const std::string &msg);

namespace Utils
{
	static inline void ImGuiSetupWin(const char* name, int x, int y, ImGuiWindowFlags flags = DefaultWinFlags)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });
		ImGui::Begin(name, 0, flags);
		ImGui::SetWindowPos(ImVec2(x, y));
	}

	static inline void ImGuiSetupPage(const char *name, int x, int y, ImGuiWindowFlags flags = DefaultWinFlags)
	{
		ImGuiSetupWin(name, x, y, flags);
		ImGui::SetWindowSize(ImVec2(SCR_W - x, SCR_H - y));
		//if (page->focused)
			//ImGui::SetWindowFocus();
		//ImGui::GetCurrentContext()->NavDisableHighlight = !focus;
	}

	static inline void ImGuiSetupPage(const IPage* page, int x, int y, ImGuiWindowFlags flags = DefaultWinFlags)
	{
		ImGuiSetupWin(page->Name.c_str(), x, y, flags);
		ImGui::SetWindowSize(ImVec2(SCR_W - x, SCR_H - y));
		//if (page->focused)
			//ImGui::SetWindowFocus();
		//ImGui::GetCurrentContext()->NavDisableHighlight = !focus;
	}

	static inline void ImGuiCloseWin()
	{
		ImGui::PopStyleVar(2);
		ImGui::End();
	}

	static inline void ImGuiSetWindowScrollable() 
	{
		if (!AnyNavButtonPressed()) {
			auto cursorLastPos = ImGui::GetCursorPosY();
			ImGui::SetCursorPos({ 0, 0 });
			ImGui::PushStyleColor(ImGuiCol_Button, { 0,0,0,0 });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0,0,0,0 });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0,0,0,0 });
			ImGui::Button("##drag", { ImGui::GetWindowSize().x, cursorLastPos });
			if (ImGui::IsItemActive())
			{
				ImVec2 drag = ImGui::GetMouseDragDelta(0);
				ImGui::SetScrollY(ImGui::GetScrollY() - drag.y);
			}
			ImGui::PopStyleColor(3);
		}
	}

	static inline bool ItemNotDragging() 
	{
		return ImGui::GetMouseDragDelta(0).y == 0;
	}

	static inline void ImGuiCenterString(const std::string& str)
	{
		auto win = ImGui::GetWindowWidth();
		auto sz = ImGui::CalcTextSize(str.c_str(), nullptr, false, win);

		ImGui::SetCursorPosX(win/ 2 - sz.x / 2);
		ImGui::TextWrapped(str.c_str());
	}

	static inline void ImGuiRightString(const std::string& str)
	{
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(str.c_str()).x - 2);
		ImGui::Text(str.c_str());
	}

	static inline bool PageLeaveFocusInput(bool AllowLeft = true)
	{
		return KeyPressed(GLFW_GAMEPAD_BUTTON_B) || KeyPressed(GLFW_GAMEPAD_BUTTON_DPAD_LEFT) && AllowLeft;
	}

	static inline void ImGuiSelectItem(bool isFocused = true, ImGuiID ID = 0)
	{
		auto win = ImGui::GetCurrentWindow();
		if (ID == 0) ID = win->DC.LastItemId;
		ImGui::SetNavID(ID, 0);
		ImGui::SetFocusID(ID, win);
		ImGui::GetCurrentContext()->NavDisableHighlight = !isFocused;
	}

#define PAGE_RESET_FOCUS \
	if (FocusEvent.Reset() && ImGui::GetFocusID() == 0) Utils::ImGuiSelectItem(true);

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
