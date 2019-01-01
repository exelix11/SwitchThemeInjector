#include "ExternalInstallPage.hpp"
#include "../ViewFunctions.hpp"
#include "../input.hpp"
#include "ThemeEntry.hpp"
#include "CfwSelectPage.hpp"

using namespace std;

ExternalInstallPage::ExternalInstallPage(std::string path) : Title("Install theme from external source",WHITE, 1000, font30), Install("Install (+)"), Reboot("Reboot (-)"), HBmenu("Exit to hbmenu (+)")
{
	Install.selected = true;
    Reboot.selected = false;
    HBmenu.selected = false;
    this->ArgEntry = new ThemeEntry(path);
    auto f = SearchCfwFolders();
	if (f.size() != 1)
		PushPage(new CfwSelectPage(f));
}

void ExternalInstallPage::Render(int X, int Y)
{	
	SDL_SetRenderDrawColor(sdl_render,45,45,45,0xff); //Switch dark bg
	SDL_RenderFillRect(sdl_render,&ScreenRect);

    if(isInstalled)
    {
        HBmenu.Render(Title.GetSize().x + Title.GetSize().w - HBmenu.GetSize().w, SCR_H - 50 - HBmenu.GetSize().h);
        Reboot.Render(Title.GetSize().x, SCR_H - 50 - Reboot.GetSize().h);
    }else
    {
        Install.Render(SCR_W/2 - Install.GetSize().w/2, SCR_H - 50 - Install.GetSize().h);
        ArgEntry->Render(SCR_W/2 - ArgEntry->GetRect().w/2, SCR_H/2 - ArgEntry->GetRect().h/2, false);
    }
    Title.Render(SCR_W / 2 - Title.GetSize().w / 2,70);
}

void ExternalInstallPage::Update()
{
    if (isInstalled)
    {
        if (kDown & KEY_PLUS)
        {
            QuitApp();
        }
        else if (kDown & KEY_MINUS)
        {
            bpcInitialize();
            bpcRebootSystem();
        }
        else if (kDown & KEY_LEFT)
        {
            HBmenu.selected = false;
            Reboot.selected = true;
        }
        else if (kDown & KEY_RIGHT)
        {
            HBmenu.selected = true;
            Reboot.selected = false;
        }
        else if (kDown & KEY_A)
        {
            if(HBmenu.selected)
            {
                QuitApp();
            }
            else if(Reboot.selected)
            {
                bpcInitialize();
                bpcRebootSystem();
            }
        }
    }else
    {
        if (kDown & KEY_PLUS)
        {
            DisplayLoading("Installing...");
            if(!ArgEntry->InstallTheme(false))
            {
                Title.SetString("Failed to install external theme");
                Title.SetColor({255, 0, 0});
            }else{
                Title.SetString("Installed theme from external source");
            }
            isInstalled = true;
            HBmenu.selected = true;
        }
    }
}




