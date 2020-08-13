#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../UI/UI.hpp"
#include "../fs.hpp"
#include <functional>

class CfwSelectPage : public IUIControlObj
{
	public:
		CfwSelectPage(const std::vector<std::string> &folders);	
		~CfwSelectPage();
		
		void Render(int X, int Y) override;
		void Update() override;
	private:		
		std::vector<std::string> Folders;
};