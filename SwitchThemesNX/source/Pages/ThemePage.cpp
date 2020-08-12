#include "ThemePage.hpp"
#include "../ViewFunctions.hpp"
#include <algorithm>
#include "../Platform/Platform.hpp"

#include "../UI/imgui/imgui_internal.h"

using namespace std;

ThemesPage* ThemesPage::Instance = nullptr;
const ImVec2 TabPageSize = { 900, 552 };

ThemesPage::ThemesPage() : 
lblPage(""),
NoThemesLbl(
	"There's nothing here, copy your themes in the themes folder on your sd and try again.\n"
	"If you do have a themes folder in your sd with themes make sure that the name is all lowercase and that you don't have the archive bit issue if you use a mac or sd corruption if you use exfat, you can find more about those on google or ask for support on discord.")
{
	if (Instance)
		throw std::runtime_error("ThemePage::Instance should not be set");
	Instance = this;

	if (UseLowMemory)
		LimitLoad = 15;

	Name = "Themes";
	lblCommands = CommandsTextNormal;

	RefreshThemesList();
}

void ThemesPage::RefreshThemesList()
{
	DisplayLoading("Loading theme list...");
	ClearSelection();
	SetPage(-1);
	CursorMemory.clear();

	ThemeFiles = fs::theme::ScanThemeFiles();
	std::sort(ThemeFiles.begin(), ThemeFiles.end());

	if (SelectOnRescanTarget != "")
	{
		SelectElementByPath(SelectOnRescanTarget);
		SelectOnRescanTarget = "";
	}
	else SetDir(fs::path::ThemesFolder);
}

void ThemesPage::SelectElementByPath(const std::string& path)
{	
	if (std::find(ThemeFiles.begin(), ThemeFiles.end(), path) == ThemeFiles.end())
		return; // File not in index

	SetDir(fs::GetPath(path));

	auto f = std::find(CurrentFiles.begin(), CurrentFiles.end(), path);
	if (f == CurrentFiles.end())
		return; // Can this ever happen ?

	size_t index = f - CurrentFiles.begin();

	int page = index / pageCount - 1;
	int pageindex = index % pageCount;

	SetPage(page, pageindex);
}

ThemesPage::~ThemesPage()
{
	Instance = nullptr;
	SetPage(-1);
}

void ThemesPage::SetDir(const string &dir)
{
	if (pageNum != -1)
		CursorMemory[CurrentDir] = tuple<int,int>(pageNum, menuIndex);

	CurrentDir = dir;
	if (!StrEndsWith(dir, "/"))
		CurrentDir += "/";
	
	CurrentFiles.clear();
	for (auto f : ThemeFiles)
		if (fs::GetPath(f) == CurrentDir)
			CurrentFiles.push_back(f);
	
	pageCount = CurrentFiles.size() / LimitLoad + 1;
	if (CurrentFiles.size() % LimitLoad == 0)
		pageCount--;

	pageNum = -1; //force setpage to reload the entries even if in the same page as the path changed
	if (CursorMemory.count(dir))
	{
		const auto& [num, index] = CursorMemory[dir];
		SetPage(num, index);
	}
	else SetPage(0);
}

void ThemesPage::SetPage(int num, int index)
{
	ImGui::NavMoveRequestCancel();
	if (pageNum != num || index != 0)
	{
		menuIndex = index;
		ResetScroll = true;
	}

	if (pageNum == num)	return;
	DisplayEntries.clear();
	
	size_t baseIndex = num * LimitLoad;
	if (num < 0 || baseIndex >= CurrentFiles.size())  
	{
		lblPage = (CurrentDir + " - Empty");
		pageNum = num;
		return;
	}
	
	int imax = CurrentFiles.size() - baseIndex;
	if (imax > LimitLoad) imax = LimitLoad;
	for (int i = 0; i < imax; i++)
		DisplayEntries.push_back(ThemeEntry::FromFile(CurrentFiles[baseIndex + i]));
	
	pageNum = num;
	auto LblPStr = CurrentDir + " - Page " + to_string(num + 1) + "/" + to_string(pageCount);
	if (SelectedFiles.size() != 0)
		LblPStr = "("+ to_string(SelectedFiles.size()) + " selected) " + LblPStr;
	lblPage = LblPStr;
	lblCommands = (SelectedFiles.size() == 0 ? CommandsTextNormal : CommandsTextSelected);
}

void ThemesPage::Render(int X, int Y)
{
	Utils::ImGuiSetupPage("ThemesPageContainer", X, Y, DefaultWinFlags | ImGuiWindowFlags_NoBringToFrontOnFocus);
	ImGui::PushFont(font25);

	if (ImGui::GetCurrentWindow()->Appearing && fs::theme::ShouldRescanThemeList())
		PushFunction([this]() { RefreshThemesList(); });

	if (DisplayEntries.size() == 0)
		ImGui::TextWrapped(NoThemesLbl.c_str());

	ImGui::SetCursorPosY(600);
	Utils::ImGuiRightString(lblPage);
	
	if (DisplayEntries.size() == 0)
		goto QUIT_RENDERING;

	ImGui::SetCursorPosY(570);
	ImGui::TextUnformatted(lblCommands.c_str());

	{
		ImGui::SetNextWindowSize(TabPageSize);
		Utils::ImGuiSetupWin("ThemesList", X, Y, DefaultWinFlags & ~ImGuiWindowFlags_NoScrollbar);
		int setNewMenuIndex = 0;
		if (ResetScroll || ImGui::GetCurrentWindow()->Appearing)
		{
			setNewMenuIndex = menuIndex;
			ImGui::NavMoveRequestCancel();
			ImGui::SetScrollY(0);
			FocusEvent.Set();
			ResetScroll = false;
		}		
		{
			int count = 0;
			for (auto& e : DisplayEntries)
			{
				bool Selected = IsSelected(e->GetPath());
				if (Selected)
					ImGui::PushStyleColor(ImGuiCol_WindowBg, 0x366e64ff);

				if (e->IsHighlighted())
					menuIndex = count;
				auto res = e->Render(Selected);

				if (Selected)
					ImGui::PopStyleColor();
				if (count == setNewMenuIndex && FocusEvent.Reset()) Utils::ImGuiSelectItem(true);
				Utils::ImGuiDragWithLastElement();

				if (res == ThemeEntry::UserAction::Preview)
					break;
				else if (res == ThemeEntry::UserAction::Enter)
					PushFunction([count, &e, this]()
						{
							if (e->IsFolder())
								SetDir(e->GetPath());
							else
							{
								if (SelectedFiles.size() == 0)
								{
										e->Install();
								}
								else
								{
									if (menuIndex != count)
										menuIndex = count;
									SelectCurrent();
								}
							}
						});

				count++;
			}
		}

		//Here scrolling is handled by the individual theme entries, Utils::ImGuiSetWindowScrollable is not needed
		Utils::ImGuiCloseWin();
	}
QUIT_RENDERING:
	ImGui::PopFont();
	Utils::ImGuiCloseWin();
}

int ThemesPage::PageItemsCount()
{
	int menuCount = CurrentFiles.size() - pageNum * LimitLoad;
	if (menuCount > LimitLoad)
		menuCount = LimitLoad;
	if (menuCount < 0) return 0;
	return menuCount;
}

inline bool ThemesPage::IsSelected(const std::string &fname)
{
	return (std::find(SelectedFiles.begin(), SelectedFiles.end(), fname) != SelectedFiles.end());
}

void ThemesPage::ClearSelection()
{
	SelectedFiles.clear();
	lblCommands = CommandsTextNormal;
}

void ThemesPage::SelectCurrent()
{
	if (DisplayEntries[menuIndex]->IsFolder() || !DisplayEntries[menuIndex]->CanInstall()) return;
	auto fname = DisplayEntries[menuIndex]->GetPath();
	auto position = std::find(SelectedFiles.begin(), SelectedFiles.end(), fname);
	if (position != SelectedFiles.end())
	{
		SelectedFiles.erase(position);
	}
	else 
	{
		SelectedFiles.push_back(fname);
	}
	lblCommands = (SelectedFiles.size() == 0 ? CommandsTextNormal : CommandsTextSelected);
}

void ThemesPage::Update()
{
	int menuCount = PageItemsCount();	
	
	if (NAV_LEFT)
		Parent->PageLeaveFocus(this);
	if (KeyPressed(GLFW_GAMEPAD_BUTTON_B))
	{
		if (CurrentDir != fs::path::ThemesFolder)
			SetDir(fs::GetParentDir(CurrentDir));
		else 
			Parent->PageLeaveFocus(this);
	}
	
	if (menuCount <= 0)
		return;

	if ((NAV_UP && menuIndex <= 0) || KeyPressed(GLFW_GAMEPAD_BUTTON_LEFT_BUMPER))
	{
		if (pageNum > 0)
		{
			SetPage(pageNum - 1);
			menuIndex = PageItemsCount() - 1;
			return;
		}
		else if (pageCount > 1)
		{
			SetPage(pageCount - 1);
			menuIndex = PageItemsCount() - 1;
			return;
		}
		else
		{
			menuIndex = PageItemsCount() - 1;
			ResetScroll = true;
		}
	}
	else if ((NAV_DOWN && menuIndex >= PageItemsCount() - 1) || KeyPressed(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER))
	{
		if (pageCount > pageNum + 1)
			SetPage(pageNum + 1);
		else if (pageNum != 0)
			SetPage(0);
		else
		{
			menuIndex = 0;
			ResetScroll = true;
		}
	}
	else if (KeyPressed(GLFW_GAMEPAD_BUTTON_Y))
	{
		if (SelectedFiles.size() == 0 && menuIndex >= 0)
			SelectCurrent();
		else {
			ClearSelection();
		}
	}
	else if (KeyPressed(GLFW_GAMEPAD_BUTTON_START) && SelectedFiles.size() != 0)
	{
		for (string file : SelectedFiles)
		{
			ThemeEntry::DisplayInstallDialog(file);
			if (!ThemeEntry::FromFile(file)->Install(false))
			{
				Dialog("Installing a theme failed, the process was cancelled");
				break;
			}
		}
		ClearSelection();
		SetPage(pageNum);		
	}
}

void ThemesPage::SelectElementOnRescan(const std::string& path)
{
	SelectOnRescanTarget = path;
}
