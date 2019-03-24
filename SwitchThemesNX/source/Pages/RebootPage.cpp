#include <switch.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../UI/UI.hpp"
#include "../fs.hpp"
#include "../input.hpp"
#include "../SwitchTools/PayloadReboot.hpp"

const SDL_Color YELLOW_WARN = {0xff,0xce,0x0a};
const SDL_Color RED_ERROR = {0xff,0x34,0x19};

class RebootPage : public IPage
{
	public:
		RebootPage() :
		DescriptionLbl("Reboot to payload allows you to reboot your console without having to inject a payload again. Currently it's supported only on atmosphere.",WHITE, 895, font30),
		ErrorLbl("This feature isn't available with your current setup, you need to use Atmosphere >= 0.8.3 and the reboot payload placed in your sd card at /atmosphere/reboot_payload.bin" ,RED_ERROR, 895, font30),
		WarningLbl("Reboot to payload is properly setup but multiple CFWs were detected on your sd card, THIS WILL CRASH IF YOU'RE NOT RUNNING ATMOSPHERE " ,YELLOW_WARN, 895, font30),
		RebootBtn("Reboot")
		{
			Name = "Reboot to payload";
			RebootBtn.selected = false;
			
			auto v = SearchCfwFolders();
			bool hasAtmos = false;
			if (std::find(v.begin(), v.end(), "/atmosphere") != v.end())
			{
				ShowError = false;
				hasAtmos = true;
			}
						
			if (!hasAtmos) return;			
			
			if (!PayloadReboot::Init())
			{				
				ShowError = true;
				return;
			}
			else CanReboot = true;
			
			if (hasAtmos && v.size() != 1)
				ShowWarning = true;
		}
		
		void Render(int X, int Y) 
		{
			int baseY = Y + 20;
			DescriptionLbl.Render(X + 20 , baseY);
			baseY += DescriptionLbl.GetSize().h + 20;
			if (ShowError)
			{
				ErrorLbl.Render(X + 20 , baseY);
				baseY += ErrorLbl.GetSize().h + 20;				
			}
			if (ShowWarning)
			{
				WarningLbl.Render(X + 20 , baseY);
				baseY += WarningLbl.GetSize().h + 20;				
			}
			if (CanReboot)
			{
				baseY += 20;
				RebootBtn.Render(X + 20, baseY);
			}
		}
		
		void Update() override
		{
			if (!CanReboot)
				Parent->PageLeaveFocus(this);			
			
			RebootBtn.selected = true;
			if (kDown & KEY_A)
			{
				PayloadReboot::Reboot();
			}
			else if (kDown & KEY_B || kDown & KEY_LEFT){
				RebootBtn.selected = false;
				Parent->PageLeaveFocus(this);
			}
		}
	private:
		bool CanReboot = false;
		bool ShowError = true;
		bool ShowWarning = false;
	
		Label DescriptionLbl;
		Label ErrorLbl;
		Label WarningLbl;
		Button RebootBtn;
};