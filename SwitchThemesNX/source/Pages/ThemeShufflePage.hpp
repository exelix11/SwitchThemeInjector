#pragma once
#include <switch.h>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../UI/UI.hpp"
#include "../fs.hpp"

class ShufflePage : public IPage
{
	public:
		ShufflePage();	
		
		void Render(int X, int Y) override;
		void Update() override;
	private:	
		void LoadShuffleState();
		
		void WriteShuffleFlag(int i);
	
		Label lblGuide;
		Label lblSortMode;
		Button btnRandom;
		Button btnCycle;
		Label lblInstalled;
		Button btnClear;
		
		int selectedIndex = 0;
		
		bool hasFocus = true;
};
