#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../UI/UI.hpp"
#include "../fs.hpp"
#include "ThemeEntry/ThemeEntry.hpp"
#include <functional>

class ExternalInstallPage : public IUIControlObj
{
	public:
		ExternalInstallPage(const std::vector<std::string> &paths);
		~ExternalInstallPage();
		
		void Render(int X, int Y) override;
		void Update() override;
	private:
		bool tooManyItems = false;
		int RenderStartIndex = 0;
		int SelectedIndex = 0;
	
		std::string Title;
		std::string Install;
        bool isInstalled = false;		
		std::vector <std::unique_ptr<ThemeEntry>> ArgEntries; 
		const u32 GRAY = 0x808080FF;
};