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
	
		Label Title;
        Button Install;
        Button Reboot;
        Button HBmenu;
        bool isInstalled = false;		
		std::vector <ThemeEntry*> ArgEntries; 
		const SDL_Color GRAY = {0x80,0x80,0x80};
};