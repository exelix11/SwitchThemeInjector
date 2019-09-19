#include "ThemePage.hpp"
#include "../ViewFunctions.hpp"
#include <algorithm>
#include "../Platform/Platform.hpp"

#include "../UI/imgui/imgui_internal.h"

using namespace std;

static constexpr int LimitLoad = 25;

ThemesPage::ThemesPage(const std::vector<std::string> &files) : 
lblPage(""),
NoThemesLbl(
	"There's nothing here, copy your themes in the themes folder on your sd and try again.\n"
	"If you do have a themes folder in your sd with themes make sure that the name is all lowercase and that you don't have the archive bit issue if you use a mac or sd corruption if you use exfat, you can find more about those on google or ask for support on discord.")
{
	Name = "Themes";
	ThemeFiles = files;
	lblCommands = CommandsTextNormal;
	std::sort(ThemeFiles.begin(), ThemeFiles.end());
	SetDir(SD_PREFIX "/themes");
}

ThemesPage::~ThemesPage()
{
	SetPage(-1);
}

void ThemesPage::SetDir(const string &dir)
{
	CurrentDir = dir;
	if (!StrEndsWith(dir, "/"))
		CurrentDir += "/";
	
	CurrentFiles.clear();
	for (auto f : ThemeFiles)
	{
		if (fs::GetPath(f) == CurrentDir)
			CurrentFiles.push_back(f);
	}
	
	pageCount = CurrentFiles.size() / LimitLoad + 1;
	if (CurrentFiles.size() % LimitLoad == 0)
		pageCount--;
	menuIndex = 0;
	SetPage(0);
}

void ThemesPage::SetPage(int num)
{
	ImGui::NavMoveRequestCancel();
	if (pageNum != num)
	{
		menuIndex = 0;
		ResetScroll = true;
	}
	for (auto i : DisplayEntries)
		delete i;
	DisplayEntries.clear();
	
	size_t baseIndex = num * LimitLoad;
	if (num < 0 || baseIndex >= CurrentFiles.size())  
	{
		lblPage = (CurrentDir + " - Empty");
		pageNum = num;
		return;
	}
	
	DisplayLoading("Loading...");
	int imax = CurrentFiles.size() - baseIndex;
	if (imax > LimitLoad) imax = LimitLoad;
	for (int i = 0; i < imax; i++)
	{
		auto entry = new ThemeEntry(CurrentFiles[baseIndex + i]);
		DisplayEntries.push_back(entry);
	}
	pageNum = num;
	auto LblPStr = CurrentDir + " - Page " + to_string(num + 1) + "/" + to_string(pageCount);
	if (SelectedFiles.size() != 0)
		LblPStr = "("+ to_string(SelectedFiles.size()) + " selected) " + LblPStr;
	lblPage = LblPStr;
	lblCommands = (SelectedFiles.size() == 0 ? CommandsTextNormal : CommandsTextSelected);
}

const int EntryW = 860;
void ThemesPage::Render(int X, int Y)
{
	Utils::ImGuiSetupPage("ThemesPageContainer", X, Y, DefaultWinFlags | ImGuiWindowFlags_NoBringToFrontOnFocus);
	ImGui::PushFont(font25);

	if (DisplayEntries.size() == 0)
		ImGui::TextWrapped(NoThemesLbl.c_str());

	ImGui::SetCursorPosY(600);
	Utils::ImGuiRightString(lblPage);
	
	if (DisplayEntries.size() == 0)
		goto QUIT_RENDERING;

	ImGui::SetCursorPosY(570);
	ImGui::Text(lblCommands.c_str());

	{
		Utils::ImGuiSetupPage(this, X, Y, DefaultWinFlags & ~ImGuiWindowFlags_NoScrollbar);
		int setNewMenuIndex = 0;
		if (ResetScroll)
		{
			setNewMenuIndex = menuIndex;
			ImGui::NavMoveRequestCancel();
			ImGui::SetScrollY(0);
			FocusEvent.Set();
			ResetScroll = false;
		}
		ImGui::SetWindowSize(TabPageArea);
		{
			int count = 0;
			for (auto& e : DisplayEntries)
			{
				bool Selected = IsSelected(e->GetPath());
				if (Selected)
					ImGui::PushStyleColor(ImGuiCol_Button, 0x366e64ff);

				if (e->IsHighlighted())
					menuIndex = count;
				auto res = e->Render(Selected);

				if (Selected)
					ImGui::PopStyleColor();
				if (count == setNewMenuIndex && FocusEvent.Reset()) Utils::ImGuiSelectItem(true);
				Utils::ImGuiDragWithLastElement();

				if (res == ThemeEntry::UserAction::Preview)
					break;
				else if (res == ThemeEntry::UserAction::Install)
					PushFunction([count, &e, this]()
						{
							if (e->IsFolder)
								SetDir(e->GetPath());
							else
							{
								if (SelectedFiles.size() == 0)
								{
									if (gamepad.buttons[GLFW_GAMEPAD_BUTTON_GUIDE])
									{
										DisplayLoading("Installing to shuffle...");
										e->InstallTheme(false, shuffle::MakeThemeShuffleDir());
									}
									else
										e->InstallTheme();
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

		Utils::ImGuiSetWindowScrollable();
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

void ThemesPage::SelectCurrent()
{
	if (DisplayEntries[menuIndex]->IsFolder) return;
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
		if (CurrentDir != SD_PREFIX "/themes/")
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
			SelectedFiles.clear();
			lblCommands = CommandsTextNormal;
		}
	}
	else if (KeyPressed(GLFW_GAMEPAD_BUTTON_START) && SelectedFiles.size() != 0)
	{
		string shuffleDir = "";
		if (gamepad.buttons[GLFW_GAMEPAD_BUTTON_GUIDE])
			shuffleDir = shuffle::MakeThemeShuffleDir();
		for (string file : SelectedFiles)
		{
			DisplayLoading("Installing " + file + "...");
			ThemeEntry t {file};
			if (!t.InstallTheme(false,shuffleDir))
			{
				Dialog("Installing a theme failed, the process was cancelled");
				break;
			}
		}
		SelectedFiles.clear();
		SetPage(pageNum);		
	}
}