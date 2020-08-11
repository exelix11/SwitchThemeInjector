#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../UI/UI.hpp"
#include "../fs.hpp"
#include "ThemeEntry/ThemeEntry.hpp"

class ThemesPage : public IPage
{
	public:
		static ThemesPage* Instance;

		ThemesPage();	
		~ThemesPage();
		
		void Render(int X, int Y) override;
		void Update() override;

		void SelectElementOnRescan(const std::string& path);
	private:
		void RefreshThemesList();
		void SelectElementByPath(const std::string& path);

		void SetDir(const std::string &dir);
		void SetPage(int num, int index = 0);
		void SelectCurrent();
		
		int PageItemsCount();
		
		std::vector<std::string> ThemeFiles;
		bool IsSelected(const std::string &fname);
		void ClearSelection();

		std::string CurrentDir;
		std::vector<std::string> CurrentFiles;
		
		std::vector<std::unique_ptr<ThemeEntry>> DisplayEntries;
		std::string lblPage;
		std::string lblCommands;
		int pageNum = -1;
		int pageCount = -1;
		
		//Will reset the scroll and force the selected item on the ui
		bool ResetScroll = false;
		int menuIndex = 0;

		std::vector<std::string> SelectedFiles;
		
		std::string NoThemesLbl;
		
		const std::string CommandsTextNormal = "A: Install theme  Y: Multiselect  L/R: Previous/Next page";
		const std::string CommandsTextSelected = "A: Add/Remove to selection  Y: Clear selection  `+`: Install selected";

		int LimitLoad = 25;

		std::unordered_map<std::string, std::tuple<int,int>> CursorMemory;

		std::string SelectOnRescanTarget;
};