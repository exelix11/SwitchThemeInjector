#pragma once
#include <switch.h>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../UI/UI.hpp"
#include "../fs.hpp"

class TextPage : public IPage
{
	public:
		TextPage(const std::string &title, const std::string &text);	
		
		void Render(int X, int Y) override;
		void Update() override;
	private:
	
		Label Text;
};

class CreditsPage : public IPage
{
	public:
		CreditsPage();	
		
		void Render(int X, int Y) override;
		void Update() override;
	private:
	
		Label creditsText;
		Label creditsText2;
		Button showHelp;
};
