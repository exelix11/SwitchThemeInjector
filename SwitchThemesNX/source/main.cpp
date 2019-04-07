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
#include "SwitchThemesCommon/NXTheme.hpp"
#include "Pages/RemoteInstallPage.hpp"
#include "Pages/ThemeShufflePage.hpp"
#include "Pages/RebootPage.cpp"

using namespace std;

u64 kDown = 0;
u64 kHeld = 0;
stack<IUIControlObj*> views;
bool doPopPage = false;
bool AppRunning = true;

IUIControlObj *ViewObj = nullptr;

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

void PushPage(IUIControlObj* page) //All pages must be dynamically allocated
{
	views.push(page);
	ViewObj = page;
}

void PushPageBlocking(IUIControlObj* page)
{
	PushPage(page);
	while (AppRunning && appletMainLoop())
	{ 
		hidScanInput();
		kDown = hidKeysDown(CONTROLLER_P1_AUTO);
		kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);
		
		ViewObj->Update();
		if (doPopPage)
		{			
			_PopPage();
			break;
		}
		ViewObj->Render(0,0);
		SDL_RenderPresent(sdl_render);		
	}
}

void ErrorFatal(const string &msg)
{
	PushPage(new FatalErrorPage(msg));
}

void Dialog(const string &msg)
{
	PushPage(new DialogPage(msg));
}

void DialogBlocking(const string &msg)
{
	PushPageBlocking(new DialogPage(msg));
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
	Dialog("Altough .nxtheme files can be INSTALLED on every firmware you still have to uninstall any theme before updating, this is because the nxtheme gets converted to an SZS when it's installed. After the update you will be able to reinstall all your themes in .nxtheme format without any compatibility issue.\n"
"(Please note that some features such as custom Settings page are available only on >= 6.X firmwares)");
	Dialog("SZS files unfortunately are illegal to share as they contain copyrighted data, that's why this tool also supports .nxtheme files. These work just like SZS but they can be freely shared and most importantly installed on every firmware");
	Dialog("Custom themes are custom SZS files that replace some files in the home menu, these files are firmware-dependent, this means that if you update your firmware while having a custom theme installed your console may not boot anymore until you manually remove the custom theme.\n"
"To remove a custom theme you either boot your CFW without LayeredFS and use this tool to uninstall it or manually delete the 0100000000001000 folder in sdcard/<your cfw folder>/titles\n"
"Custom themes CANNOT brick your console because they're installed only on the sd card");
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

// Note that CfwFolder is set after the constructor of any page pushed before CheckCFWDir is called, CfwFolder shouldn't be used until the theme is actually being installed
void CheckCFWDir()
{
	auto f = SearchCfwFolders();
	if (f.size() != 1)
		PushPage(new CfwSelectPage(f));
}

vector<string> GetArgsInstallList(int argc, char**argv)
{
	int i;
	string key = "installtheme=";
	string pathss;
	std::vector<std::string> paths;
	for (i=1; i< argc; i++)
	{
		string argvs(argv[i]);
		auto pos = argvs.find(key);
		size_t index;
		while (true)
		{
			index = argvs.find("(_)");
    		if (index == std::string::npos) break;
    		argvs.replace(index, 3, " ");
		}
		if (pos != std::string::npos)
			pathss = argvs.substr(pos + 13);
		
		if (!pathss.empty())
		{
    		string path;
    		stringstream stream(pathss);
    		while(getline(stream, path, ',')){
				paths.push_back(path); 
			}
		}
 	}
	return paths;
}	

std::string SystemVer = "";
void SetupSysVer()
{
	setsysInitialize();
	SetSysFirmwareVersion firmware;
	auto res = setsysGetFirmwareVersion(&firmware);
	if (R_FAILED(res))
	{
		ErrorFatal("Could not get sys ver res=" + to_string(res));
		return;
	}
	if (firmware.major <= 5)
	{
		ThemeTargetToName = ThemeTargetToName5X;
		ThemeTargetToFileName = ThemeTargetToFileName6X; //The file names are the same
	}
	else //6.X
	{
		ThemeTargetToName = ThemeTargetToName6X;
		ThemeTargetToFileName = ThemeTargetToFileName6X;
	}
	NXTheme_FirmMajor = firmware.major;
	SystemVer = to_string(firmware.major) + "." + to_string(firmware.minor) + "." + to_string(firmware.micro);
	setsysExit();
}

int main(int argc, char **argv)
{
    romfsInit();
	SdlInit();
	FontInit();
	socketInitializeDefault();
	
	std::set_unexpected (MyUnexpected);
	
	SetupSysVer();
	bool ThemesFolderExists = CheckThemesFolder();
	NcaDumpPage::CheckHomeMenuVer();

	if (envHasArgv() && argc > 1)
	{
		auto paths = GetArgsInstallList(argc,argv);
		if (paths.size() == 0)
			goto APP_QUIT;
		
		PushPage(new ExternalInstallPage(paths));	
		CheckCFWDir();		
		AppMainLoop();
		
		goto APP_QUIT;
	}	

	
	{
		TabRenderer *t = new TabRenderer();
		PushPage(t);
		
		if (!ThemesFolderExists)
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
		RebootPage *reboot = new RebootPage();
		t->AddPage(reboot);
		QuitPage *q = new QuitPage();
		t->AddPage(q);
		
		CheckCFWDir();
		
		AppMainLoop();
		
		delete p;
		delete up;
		delete dp;
		delete rmi;
		delete sf;
		delete credits;
		delete q;
	}
	
APP_QUIT:

	while (views.size() != 0)
	{
		delete views.top();
		views.pop();
	}
	
	socketExit();
	FontExit();
	SdlExit();
	romfsExit();
	
    return 0;
}