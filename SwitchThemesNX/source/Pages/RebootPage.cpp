#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../UI/UI.hpp"
#include "../fs.hpp"

#include "../Platform/Platform.hpp"
#include "../ViewFunctions.hpp"

class RebootPage : public IPage
{
	public:
		RebootPage()
		{
			Name = "Reboot";
		}
		
		void Render(int X, int Y) 
		{
			Utils::ImGuiSetupPage(this, X, Y);
			ImGui::PushFont(font30);
			ImGui::SetCursorPos({ 5, 10 });

			ImGui::TextUnformatted("Rebooting your console will apply the changes you made.");
			ImGui::TextWrapped("This is a shortcut to the system's reboot button. If your CFW doesn't provide reboot to payload you will need a way to inject a payload from RCM.");
			if (ImGui::Button("Reboot"))
			{
				PlatformReboot();
			}
			PAGE_RESET_FOCUS;
			
			ImGui::PopFont();
			Utils::ImGuiCloseWin();
		}
		
		void Update() override
		{
			if (Utils::PageLeaveFocusInput())
				Parent->PageLeaveFocus(this);
		}
};