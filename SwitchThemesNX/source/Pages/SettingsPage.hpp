#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../UI/UI.hpp"
#include "../fs.hpp"

namespace Settings {
	extern bool UseAnimations;
	extern bool UseIcons;
	extern bool UseCommon;
};

class SettingsPage : public IPage
{
	public:
		SettingsPage();	
		
		void Render(int X, int Y) override;
		void Update() override;
	private:	
		void LoadShuffleState();
		void WriteShuffleFlag(int i);
		
		std::string lblGuide;
		std::string lblInstalled;

		int shuffleValue = 0;
		bool IsLayoutBlockingLeft = false;
};
