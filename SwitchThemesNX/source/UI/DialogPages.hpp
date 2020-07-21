#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../SwitchThemesCommon/MyTypes.h"
#include "UI.hpp"
#include "../fs.hpp"
#include <functional>

class LoadingOverlay : public IUIControlObj
{	
	public:
		LoadingOverlay(std::initializer_list<std::string> lines) : _lines(lines)
		{

		}

		LoadingOverlay(const std::string& line)
		{
			_lines.push_back(line);
		}

		~LoadingOverlay() override {};

		void Render(int X, int Y) override;
		void Update() override {}
	private:
		std::vector<std::string> _lines;
};

class DialogPage : public IUIControlObj
{
	public:
		DialogPage(const std::string &msg);	
		DialogPage(const std::string &msg, const std::string &buttonMsg);	
		~DialogPage() override {};

		void Render(int X, int Y) override;
		void Update() override;
	private:
		std::string text;
		std::string btn;
};

class YesNoPage : public IUIControlObj
{
	public:
		static bool Ask(const std::string &msg);
	
		YesNoPage(const std::string &msg, bool *outRes);	
		~YesNoPage() override {};

		void Render(int X, int Y) override;
		void Update() override;
		
	private:
		bool *result;	
		std::string text;
};