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
#include "Pages/ExternalInstallPage.hpp"
#include "ViewFunctions.hpp"
#include "SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "Pages/RemoteInstallPage.hpp"
#include "Pages/ThemeShufflePage.hpp"

using namespace std;

u64 kDown = 0;
u64 kHeld = 0;
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
		kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);
		
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
	Dialog("You can find some themes in the subreddit /r/NXThemes and in the Qcean discord (invite: CUnHJgb) where you can also ask for support. \n"
"To make your own themes download the windows app at : https://git.io/fpxAS\n"
"Or use the online theme editor at: https://exelix11.github.io/SwitchThemeInjector/\n"
"\n"
"That's all, have fun with custom themes :)");
	Dialog("Altough .nxtheme files can be INSTALLED on every firmware you still have to uninstall the theme before updating, this is because the nxtheme gets converted to an SZS when it's installed. This means that every time you change your firmware you will have to do again the NCA dump procedure. But after that you will be able to reinstall all your themes in .nxtheme format without any compatibility issue.\n"
"(Please note that some features such as custom Settings page are available only on >= 6.X firmwares)");
	Dialog("SZS files unfortunately are illegal to share as they contain copyrighted data, that's why this tool also supports .nxtheme files. These work just like SZS but they can be freely shared and most importantly installed on every firmware, all you have to do to use them is to follow a simple guide, read more in the \"Extract home menu\" tab.\n"
"If you are familiar with Auto-Theme -the old way of legally sharing custom themes- you may know that the old guide was not easy, the procedure has been completely reworked, this time dumping the necessary data takes just 5 minutes and doesn't involve any complex operation.");
	Dialog("Custom themes are custom SZS files that replace some files in the home menu, these files are firmware-dependent, this means that if you update your firmware while having a custom theme installed your console may not boot anymore until you manually remove the custom theme.\n"
"To remove a custom theme you either boot your CFW without LayeredFS and use this tool to uninstall it or manually delete the 0100000000001000 folder in sdcard/<your cfw folder>/titles\n"
"Custom themes CANNOT birck your console because they're installed only on the sd card");
	if (WelcomeScr)
		Dialog("Welcome to NXThemes Installer " + VersionString + "!\n\nThese pages contains some important informations, it's recommended to read them carefully.\nThis will only show up once, you can read it again from the Credits tab." );
}

void MyUnexpected () {
	DisplayLoading("There was an unexpected exception, close this app with the home button");
	while (1)
	{
		svcSleepThread(10000000000L);
	}
}

int main(int argc, char **argv)
{
    romfsInit();
	SdlInit();
	FontInit();
	socketInitializeDefault();
	
	std::set_unexpected (MyUnexpected);

	if (envHasArgv() && argc > 1)
	{
		int i;
		string key = "installtheme=";
		string pathss;
		for (i=1; i< argc; i++)
		{
			string argvs(argv[i]);
			auto pos = argvs.find(key);
			if (pos != std::string::npos)
				pathss = argvs.substr(pos + 13);
			
			if (!pathss.empty())
			{
				size_t index;
				while (true)
				{
					index = argvs.find("(_)");
     				if (index == std::string::npos) break;
     				argvs.replace(index, 3, " ");
				}
				std::vector<std::string> paths;
    			string path;
    			stringstream stream(pathss);
    			while(getline(stream, path, ',')){
					paths.push_back(path); 
				}
				PushPage(new ExternalInstallPage(paths));
				auto f = SearchCfwFolders();
				if (f.size() != 1)
					PushPage(new CfwSelectPage(f));
				AppMainLoop();
				QuitApp();
			}
 		}
	}

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
	RemoteInstallPage *rmi = new RemoteInstallPage();
	t->AddPage(rmi);
	ShufflePage *sf = new ShufflePage();
	t->AddPage(sf);
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
	delete rmi;
	delete sf;
	delete credits;
	delete q;
	
	socketExit();
	FontExit();
	SdlExit();
	romfsExit();
	
    return 0;
}