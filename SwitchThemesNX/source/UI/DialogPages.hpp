#pragma once
#include <switch.h>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "UI.hpp"
#include "../fs.hpp"
#include <functional>

class LoadingOverlay : public IUIControlObj
{	
	public:
		LoadingOverlay(const std::string &msg);	

		void Render(int X, int Y) override;
		void Update() override;
	private:
		Button text;
};

class FatalErrorPage : public IUIControlObj
{
	public:
		FatalErrorPage(const std::string &msg);	

		void Render(int X, int Y) override;
		void Update() override;
	private:
		Label text;
};

class DialogPage : public IUIControlObj
{
	public:
		DialogPage(const std::string &msg);	
		DialogPage(const std::string &msg, const std::string &buttonMsg);	

		void Render(int X, int Y) override;
		void Update() override;
	private:
		Label text;
		Label Btn;
};

class YesNoPage : public IUIControlObj
{
	public:
		static bool Ask(const std::string &msg);
	
		YesNoPage(const std::string &msg, bool *outRes);	

		void Render(int X, int Y) override;
		void Update() override;
		
	private:
		bool *result;	
		Label text;
		Button btnYes;
		Button btnNo;
};