#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../UI/UI.hpp"
#include "../fs.hpp"
#include "../SwitchTools/PayloadReboot.hpp"

#include "../Platform/Platform.hpp"
#include "../ViewFunctions.hpp"

const u32 YELLOW_WARN = 0xffce0aff;
const u32 RED_ERROR = 0xff3419ff;

class RebootPage : public IPage
{
	public:
		RebootPage() :
		DescriptionLbl("Reboot to payload allows you to reboot your console without having to inject a payload again. Currently it's supported only on atmosphere."),
		ErrorLbl("This feature isn't available with your current setup, you need to use Atmosphere >= 0.8.3 and the reboot payload placed in your sd card at /atmosphere/reboot_payload.bin" ),
		WarningLbl("Reboot to payload is properly setup but multiple CFWs were detected on your sd card, this may not work if you're not running atmosphere "),
		RebootBtn("Reboot")
		{
			Name = "Reboot to payload";
			
			auto v = fs::SearchCfwFolders();
			bool hasAtmos = false;
			if (std::find(v.begin(), v.end(), SD_PREFIX ATMOS_DIR ) != v.end())
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
			Utils::ImGuiSetupPage(this, X, Y);
			ImGui::PushFont(font30);
			ImGui::SetCursorPos({ 5, 10 });

			ImGui::TextWrapped(DescriptionLbl.c_str());
			if (ShowError)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, RED_ERROR);
				ImGui::TextWrapped(ErrorLbl.c_str());
				ImGui::PopStyleColor();
			}
			if (ShowWarning)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, YELLOW_WARN);
				ImGui::TextWrapped(WarningLbl.c_str());
				ImGui::PopStyleColor();
			}
			if (CanReboot)
			{
				if (ImGui::Button(RebootBtn.c_str()))
				{
					PayloadReboot::Reboot();
				}
				PAGE_RESET_FOCUS
			}
			ImGui::PopFont();
			Utils::ImGuiCloseWin();
		}
		
		void Update() override
		{
			if (!CanReboot)
				Parent->PageLeaveFocus(this);			
			
			else if (Utils::PageLeaveFocusInput()){
				Parent->PageLeaveFocus(this);
			}
		}
	private:
		bool CanReboot = false;
		bool ShowError = true;
		bool ShowWarning = false;
	
		std::string DescriptionLbl;
		std::string ErrorLbl;
		std::string WarningLbl;
		std::string RebootBtn;
};