#pragma once
#include <switch.h>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../UI/UI.hpp"
#include "../fs.hpp"

class NcaDumpPage : public IPage
{
	public:
		NcaDumpPage();	
		
		void Render(int X, int Y) override;
		void Update() override;
		
		static void CheckHomeMenuVer();
	private:
	
		Label guideText;
		Button dumpNca;
};