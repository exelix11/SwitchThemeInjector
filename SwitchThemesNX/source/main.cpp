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


int main(int argc, char **argv)
{			
    romfsInit();
	SdlInit();
	FontInit();
	
	TabRenderer *t = new TabRenderer();
	PushPage(t);
	
	CheckThemesFolder();
	auto ThemeFiles = GetThemeFiles();
	
	ThemesPage *p = new ThemesPage(ThemeFiles);
	t->AddPage(p);
	UninstallPage *up = new UninstallPage();
	t->AddPage(up);
	NcaDumpPage *dp = new NcaDumpPage();
	t->AddPage(dp);
	TextPage *credits = new TextPage("Credits",
	"NXThemes installer by exelix " + VersionString + " Core Ver." + SwitchThemesCommon::CoreVer +
	"\nhttps://github.com/exelix11/SwitchThemeInjector\n\n"
	"Thanks to:\n Syroot for BinaryData lib\n AboodXD for Bntx editor and sarc lib");
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