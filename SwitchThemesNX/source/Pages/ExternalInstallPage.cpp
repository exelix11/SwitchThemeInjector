#include "ExternalInstallPage.hpp"
#include "../ViewFunctions.hpp"
#include "../input.hpp"
#include "ThemeEntry.hpp"
#include "CfwSelectPage.hpp"

using namespace std;

ExternalInstallPage::ExternalInstallPage(std::vector <std::string> paths) : Title("Install theme from external source",WHITE, 1000, font30), Install("Install (+)"), Reboot("Reboot (-)"), HBmenu("Exit to hbmenu (+)"), tooManyTxt("Too many themes to display all.", GRAY, false)
{
	Install.selected = true;
    Reboot.selected = false;
    HBmenu.selected = false;
    for (int i=0; i < (int)paths.size()-1; i++)
    {
        this->ArgEntries.push_back(new ThemeEntry(paths[i]));
    }
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
        int rectStartY = 80;
        for (int i=0; i < (int)ArgEntries.size()-1; i++)
        {
            SDL_Rect EntryRect = ArgEntries[i]->GetRect();
            int rectY = EntryRect.h * (i+1) + rectStartY;
            if(rectY + EntryRect.h < Install.GetSize().y)
            {
                ArgEntries[i]->Render(SCR_W/2 - EntryRect.w/2, SCR_H/2 - EntryRect.h/2, false);
            }
            else{
                tooManyItems = true;
            }
        }
    }
    Title.Render(SCR_W / 2 - Title.GetSize().w / 2,70);
    if(tooManyItems)
    {
        tooManyTxt.Render(SCR_W / 2 - Title.GetSize().w / 2,72+Title.GetSize().h);
    }
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
            bool installSuccess = true;
            for (int i=0; i < (int)ArgEntries.size()-1; i++)
            {
                if(!ArgEntries[i]->InstallTheme(false)) installSuccess = false;
            }
            if(!installSuccess)
            {
                Title.SetString("Theme(s) may have failed to install");
                Title.SetColor({255, 0, 0});
            }else{
                Title.SetString("Installed theme from external source");
            }
            isInstalled = true;
            HBmenu.selected = true;
        }
    }
}




