#pragma once
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
		TextPage(const char* title, const char* text);

		void Render(int X, int Y) override;
		void Update() override;
	private:
	
		std::string Text;
		const char* c_str;
};

class CreditsPage : public IPage
{
	public:
		CreditsPage();	
		
		void Render(int X, int Y) override;
		void Update() override;
	private:
	
		std::string creditsText;
		std::string creditsText2;
		bool IsLayoutBlockingLeft = false;
};
