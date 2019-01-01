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
		ExternalInstallPage(std::string path);
		
		void Render(int X, int Y) override;
		void Update() override;
	private:
		Label Title;
        Button Install;
        Button Reboot;
        Button HBmenu;
        bool isInstalled = false;
        ThemeEntry *ArgEntry;
};