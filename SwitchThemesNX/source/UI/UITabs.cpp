#include "UI.hpp"
#include "../ViewFunctions.hpp"
#include "../Version.hpp"
#include "../Platform/Platform.hpp"

using namespace std;

/*
18					18
--330  14  900		--
  ----|  |-----------

*/

const int BorderPadding = 20;

// X Y width height -> x y z w
const ImVec4 BottomRect = { 0, SCR_H - 67, SCR_W, 67 };
const ImVec4 TopRect = { 0, 0, SCR_W, 76 };
const ImVec4 SideRect = { 0, TopRect.w, 330, BottomRect.y };

#define TopLineLen 1200
#define SideLineLen 510

const ImVec2 TopLineSz = { TopLineLen,1 };
const ImVec2 SideLineSz = { 1,SideLineLen };

const ImVec2 TopLine = { SCR_W / 2 - TopLineLen / 2, TopRect.w + 1 };
const ImVec2 BottomLine = { SCR_W / 2 - TopLineLen / 2, BottomRect.y - 1 };
const ImVec2 SideLine = { SideRect.z + 1, (SCR_H) / 2 - SideLineLen / 2 };

void TabRenderer::Render(int X, int Y)
{
	ImGui::SetNextWindowSize(ImVec2(SideRect.z, SideRect.w));
	Utils::ImGuiSetupWin("TabRenderer", 0, 0, DefaultWinFlags);

	ImGui::GetCurrentWindow()->DrawList->AddRectFilledMultiColor({ SideRect.x, SideRect.y }, { (SideRect.x + SideRect.z), (SideRect.y + SideRect.w) / 2 }, 0xff2d2d2d, 0xff2d2d2d, 0xff353535, 0xff353535);
	ImGui::GetCurrentWindow()->DrawList->AddRectFilledMultiColor({ SideRect.x, (SideRect.y + SideRect.w) / 2 }, { (SideRect.x + SideRect.z), (SideRect.y + SideRect.w) }, 0xff353535, 0xff353535, 0xff2d2d2d, 0xff2d2d2d);

	if (!ControlHasFocus && !ImGui::IsWindowFocused())
		ImGui::SetWindowFocus();

	ImGui::PushFont(font30);

	ImGui::PushStyleColor(ImGuiCol_Button, { 0,0,0,0 });
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0,0.5 });
	const float BaseLabelY = TopRect.w + 15;
	const float BaseLabelX = TopLine.x + 5;
	ImGui::SetCursorPos({ BaseLabelX, BaseLabelY });
	int count = 0;
	static int selectedIndex;
	for (const IPage *page : Pages)
	{
		ImGui::SetCursorPosX(BaseLabelX);
		float CursorY; //Used to draw selection marker
		
		bool CurrentSelected = (page == CurrentControl);
		if (CurrentSelected)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::Highlight); //Font color
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 12);
			CursorY = ImGui::GetCursorPosY();
		}

		if (ImGui::Button(page->Name.c_str(), ImVec2(250,0)))
		{
			SetFocused(count);
			selectedIndex = count;
		}

		if (ImGui::GetCurrentWindow()->Appearing && count == selectedIndex)
			Utils::ImGuiSelectItem();

		if (CurrentSelected)
		{
			ImGui::PopStyleColor();
			ImGui::GetCurrentWindow()->DrawList->AddRectFilled({ BaseLabelX, CursorY }, { BaseLabelX + 4, ImGui::GetCursorPosY() - 10}, 0xffc9ff00);
		}

		if (!ControlHasFocus && GImGui->NavId == ImGui::GetID(page->Name.c_str()))
			selectedIndex = count;

		++count;
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
	}
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
	
	if (!ControlHasFocus)
		CurrentControl = Pages[selectedIndex];

	auto dList = ImGui::GetOverlayDrawList();

	dList->AddText(font40, 40, { 21,21 }, 0xffffffff, Title.c_str());

	dList->AddRectFilled(TopLine, TopLine + TopLineSz, 0xffffffff);
	dList->AddRectFilled(BottomLine, BottomLine + TopLineSz, 0xffffffff);
	//dList->AddRectFilled(SideLine, SideLine + SideLineSz, 0xffffffff);
	
	ImGui::PopFont();
	Utils::ImGuiCloseWin();

	if (ControlHasFocus && ImGui::IsWindowFocused())
		SetFocused(selectedIndex);

	if (CurrentControl)
		CurrentControl->Render((int)SideRect.z + 30, (int)TopRect.w + 14);
}

TabRenderer::TabRenderer() :
Title("NXThemes Installer " + Version::Name)
{
	CurrentControl = nullptr;
}

void TabRenderer::AddPage(IPage* page) 
{
	page->Parent = this;
	page->FocusEvent.Reset();
	Pages.push_back(page);
	if (!CurrentControl)
		CurrentControl = page;
}

void TabRenderer::RemoveAt(int id)
{
	if (Pages[id] == CurrentControl)
		PageLeaveFocus(Pages[id]);
	Pages.erase(Pages.begin() + id);
}

IPage* TabRenderer::At(int id)
{
	return Pages[id];
}

void TabRenderer::PageLeaveFocus(IPage *page)
{
	ImGui::NavMoveRequestCancel();
	ControlHasFocus = false;
}

void TabRenderer::Update()
{
	if (CurrentControl && ControlHasFocus)
	{
		CurrentControl->Update();
		return;
	}
	if (Pages.size() == 0)
		return;
	else if (NAV_RIGHT)
	{
		auto res = std::find(Pages.begin(), Pages.end(), CurrentControl);
		if (res != Pages.end())
			SetFocused(res - Pages.begin());
	}
}

void TabRenderer::SetFocused(int id)
{
	if (CurrentControl)
		CurrentControl->FocusEvent.Reset();

	CurrentControl = Pages[id];
	ImGui::SetWindowFocus(CurrentControl->Name.c_str());
	CurrentControl->FocusEvent.Set();
	ControlHasFocus = true;
}

