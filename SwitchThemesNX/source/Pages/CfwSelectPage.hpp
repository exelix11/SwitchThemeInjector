#pragma once
#include <switch.h>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../UI/UI.hpp"
#include "../fs.hpp"
#include <functional>

class CfwSelectPage : public IUIControlObj
{
	public:
		CfwSelectPage(std::vector<std::string> &folders);	
		~CfwSelectPage();
		
		void Render(int X, int Y) override;
		void Update() override;
	private:
		void SetPage(int num);
		int PageItemsCount();
		
		std::vector<std::string> Folders;
		std::vector<Label*> DisplayEntries;
		Label Title;
		int pageNum = -1;
		int pageCount = -1;
		int menuIndex = 0;
};