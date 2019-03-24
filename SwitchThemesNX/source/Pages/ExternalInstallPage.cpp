#include "ExternalInstallPage.hpp"
#include "../ViewFunctions.hpp"
#include "../input.hpp"
#include "ThemeEntry.hpp"
#include "CfwSelectPage.hpp"
#include "../SwitchTools/PayloadReboot.hpp"

using namespace std;

ExternalInstallPage::ExternalInstallPage(const vector<string> &paths) :
Title("Install theme from external source",WHITE, 1000, font30),
Install("Press + to install, B to cancel"), Reboot("Reboot (-)"), HBmenu("Exit to hbmenu (+)")
{
	Install.selected = false;
    Reboot.selected = false;
    HBmenu.selected = false;
    for (int i=0; i < (int)paths.size(); i++)
    {
        this->ArgEntries.push_back(new ThemeEntry(paths[i]));
    }
}

ExternalInstallPage::~ExternalInstallPage()
{
	for (int i=0; i < (int)ArgEntries.size(); i++)
			delete ArgEntries[i];
	ArgEntries.clear();
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
        int rectStartY = 70;
        for (int i=RenderStartIndex; i < (int)ArgEntries.size(); i++)
        {
            SDL_Rect EntryRect = ArgEntries[i]->GetRect();
            int rectY = (EntryRect.h + 2) * ((i-RenderStartIndex)+1) + rectStartY;
            if(rectY + EntryRect.h < Install.GetSize().y)
            {
                ArgEntries[i]->Render(SCR_W/2 - EntryRect.w/2, rectY, i == SelectedIndex);
				if ((kHeld & KEY_L) && ArgEntries[SelectedIndex]->HasPreview()) return; //for preview
            }
            else
			{
                tooManyItems = true;
				break;
            }
			tooManyItems = false;
        }
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
				if (PayloadReboot::Init())
				{
					PayloadReboot::Reboot();
				}
				else
				{
					bpcInitialize();
					bpcRebootSystem();
				}
			}
        }
    }
	else
    {
		if (kDown & KEY_DOWN)
		{
			if (tooManyItems)
				RenderStartIndex++;
			if (SelectedIndex + 1 < ArgEntries.size())
				SelectedIndex++;
		}
		if (kDown & KEY_UP)
		{
			if (RenderStartIndex > 0)
				RenderStartIndex--;
			if (SelectedIndex > 0)
				SelectedIndex--;
		}

		
        if (kDown & KEY_PLUS)
        {
            DisplayLoading("Installing...");
            bool installSuccess = true;
            for (int i=0; i < (int)ArgEntries.size(); i++)
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
		else if (kDown & KEY_B)
		{
			QuitApp();
		}
    }
}




