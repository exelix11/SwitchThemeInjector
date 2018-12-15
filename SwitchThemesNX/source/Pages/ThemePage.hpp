#pragma once
#include <switch.h>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../UI/UI.hpp"
#include "../fs.hpp"
#include "ThemeEntry.hpp"

class ThemesPage : public IPage
{
	public:
		ThemesPage(const std::vector<std::string> &files);	
		~ThemesPage();
		
		void Render(int X, int Y) override;
		void Update() override;
	private:
		void SetDir(const std::string &dir);
		void SetPage(int num);
		int PageItemsCount();
		
		std::vector<std::string> ThemeFiles;
		
		std::string CurrentDir;
		std::vector<std::string> CurrentFiles;
		
		std::vector<ThemeEntry*> DisplayEntries;
		Label lblPage;
		int pageNum = -1;
		int pageCount = -1;
		int menuIndex = 0;
		
		Label NoThemesLbl;
};