#include "CfwSelectPage.hpp"
#include "../ViewFunctions.hpp"
#include "../UI/UIManagement.hpp"

using namespace std;

CfwSelectPage::CfwSelectPage(const vector<string>& folders) : Folders(folders) {}

CfwSelectPage::~CfwSelectPage()
{

}

static const int BtnWidth = 500;
static const int XCursorBtn = SCR_W / 2 - BtnWidth / 2;

void CfwSelectPage::Render(int X, int Y)
{
	Utils::ImGuiSetupPageFullscreen("CfwSelectPage", 10, 10);
	ImGui::SetWindowFocus();

	if (Folders.size() == 0)
	{
		ImGui::PushFont(font30);
		Utils::ImGuiCenterString("Couldn't find any cfw folder.");
		ImGui::PopFont();
		ImGui::NewLine();
		ImGui::TextWrapped(
			"Make sure you have either the \"atmosphere\", \"reinx\" or \"sxos\" folder in the root of your sd card.\n\n"
			"Some cfws don't create this folder automatically so you should do it manually.\n"
			"If you do have the cfw folder but still see this screen make sure it's written correctly, without spaces and all lowercase.");
		
	}	
	else {
		Utils::ImGuiCenterString("Multiple cfw folders detected, which one do you want to use ?");

		ImGui::PushFont(font30);
		ImGui::SetCursorPos({ (float)XCursorBtn, ImGui::GetCursorPosY() + 30 });

		int count = 0;
		for (const auto& e : Folders)
		{
			ImGui::SetCursorPosX((float)XCursorBtn);
			if (ImGui::Button(e.c_str(), { BtnWidth, 50 }))
			{
				fs::cfw::SetFolder(e);
				PopPage(this);
			}
			count++;
		}

		ImGui::PopFont();
	}

	ImGui::NewLine();
	Utils::ImGuiCenterString("if your cfw isn't supported open an issue on Github.");
	if (Utils::ImGuiCenterButton("Close this application"))
		App::Quit();

	Utils::ImGuiSetWindowScrollable();
	Utils::ImGuiCloseWin();
}

void CfwSelectPage::Update()
{
	if (gamepad.buttons[GLFW_GAMEPAD_BUTTON_START])
		App::Quit();
}


