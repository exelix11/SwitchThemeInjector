#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <stdio.h>
#include <switch.h>
#include <filesystem>
#include <stack>
#include "Pages/ThemePage.hpp"
#include "Pages/CfwSelectPage.hpp"
#include "Pages/UninstallPage.hpp"
#include "Pages/NcaDumpPage.hpp"
#include "Pages/TextPage.hpp"
#include "ViewFunctions.hpp"
#include "SwitchThemesCommon/SwitchThemesCommon.hpp"

using namespace std;

u64 kDown = 0;
stack<IUIControlObj*> views;
bool doPopPage = false;
bool AppRunning = true;

IUIControlObj *ViewObj = nullptr;

void PushPage(IUIControlObj* page) //All pages must be dynamically allocated
{
	views.push(page);
	ViewObj = page;
}

void PopPage()
{
	doPopPage = true;
}

void _PopPage()
{
	doPopPage = false;
	delete views.top();
	views.pop();
	if (views.size() == 0)
	{
		ErrorFatal("Can't pop last page");
		return;
	}
	ViewObj = views.top();
}

void ErrorFatal(const string &msg)
{
	PushPage(new FatalErrorPage(msg));
}

void Dialog(const string &msg)
{
	PushPage(new DialogPage(msg));
}

void DisplayLoading(const string &msg)
{
	LoadingOverlay o(msg);
	o.Render(0,0);
	SDL_RenderPresent(sdl_render);		
}

void QuitApp()
{
	AppRunning = false;
}

void AppMainLoop()
{
	while (AppRunning && appletMainLoop())
	{ 
		hidScanInput();
		kDown = hidKeysDown(CONTROLLER_P1_AUTO);
		
		ViewObj->Update();
		if (doPopPage)
			_PopPage();
		ViewObj->Render(0,0);
		SDL_RenderPresent(sdl_render);		
	}
}

class QuitPage : public IPage
{
	public:
		QuitPage()
		{
			Name = "Quit";
		}	
		
		void Render(int X, int Y) override {}
		
		void Update() override
		{
			QuitApp();
		}
};

void ShowFirstTimeHelp(bool WelcomeScr)
{	
//these are shown from the last to the first
	Dialog("That's all, have fun with custom themes :)");
	Dialog("Altough .nxtheme files can be INSTALLED on every firmware you still have to uninstall the theme before updating, this is because the nxtheme gets converted to an SZS when it's installed. This means that every time you change your firmware you will have to do again the NCA dump procedure. But after that you will be able to reinstall all your themes in .nxtheme format without any compatibility issue.\n(Please note that some features such as custom Settings page are available only on >= 6.X firmwares)");
	Dialog("SZS files unfortunately are illegal to share as they contain copyrighted data, that's why this tool also supports .nxtheme files that are just like SZS but they can be freely shared and installed on every firmware, all you have to do is to follow a simple guide, read more in the \"Dump NCA\" tab.\nIf you are familiar with Auto-Theme, the old way of legally sharing custom themes you may know that the old guide was not easy, the procedure has been completely reworked, this time the dumping the necessary data takes just 5 minutes and doesn't involve any complex operation.");
	Dialog("Custom themes are custom SZS files that replace the home menu, these files are firmware-dependent, this means that if you update your firmware while having a custom theme installed your console may not boot anymore untill you manually remove the custom theme.\nTo remove a custom theme you either boot your CFW without LayeredFS and use this tool to uninstall it or manually delete the 0100000000001000 folder in titles in your cfw folder on the sd.");
	if (WelcomeScr)
		Dialog("Welcome to NXThemes Installer " + VersionString + "!\n\nThese pages contains some important informations about the usage, it's recommended to read them carefully.\nThis will only show up once, you can read it again from the Credits tab." );
}

int main(int argc, char **argv)
{			
    romfsInit();
	SdlInit();
	FontInit();
	
	TabRenderer *t = new TabRenderer();
	PushPage(t);
	
	if (!CheckThemesFolder())
		ShowFirstTimeHelp(true);
	
	auto ThemeFiles = GetThemeFiles();
	
	ThemesPage *p = new ThemesPage(ThemeFiles);
	t->AddPage(p);
	UninstallPage *up = new UninstallPage();
	t->AddPage(up);
	NcaDumpPage *dp = new NcaDumpPage();
	t->AddPage(dp);
	CreditsPage *credits = new CreditsPage();
	t->AddPage(credits);
	QuitPage *q = new QuitPage();
	t->AddPage(q);
	
	{
		auto f = SearchCfwFolders();
		if (f.size() != 1)
			PushPage(new CfwSelectPage(f));
	}	
	
	AppMainLoop();
	
	while (views.size() != 0)
	{
		delete views.top();
		views.pop();
	}
	
	delete p;
	delete up;
	delete dp;
	delete credits;
	delete q;
	
	FontExit();
	SdlExit();
	romfsExit();
	
    return 0;
}