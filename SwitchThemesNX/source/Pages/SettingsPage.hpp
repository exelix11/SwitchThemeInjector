#pragma once
#include <switch.h>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../UI/UI.hpp"
#include "../fs.hpp"

class SettingsPage : public IPage
{
	public:
		SettingsPage();	
		
		void Render(int X, int Y) override;
		void Update() override;
	private:	
		void LoadShuffleState();
		void UpdateAnimState();
		void WriteShuffleFlag(int i);
		
		void ClearBtnState();	
	
		Label lblAnimSupport;
		Button btnAnimEnable;
	
		Label lblShuffleSettings;
		Label lblGuide;
		Label lblSortMode;
		Button btnRandom;
		Button btnCycle;
		Label lblInstalled;
		Button btnClear;
		
		int selectedIndex = 0;
		
		bool hasFocus = true;
};
