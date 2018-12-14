#pragma once
#include <switch.h>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../UI/UI.hpp"
#include "../fs.hpp"

class UninstallPage : public IPage
{
	public:
		UninstallPage();	
		
		void Render(int X, int Y) override;
		void Update() override;
	private:
	
		Label lblText;
		Button btn;
};