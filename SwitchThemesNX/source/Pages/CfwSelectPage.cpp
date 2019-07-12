#include "CfwSelectPage.hpp"
#include "../ViewFunctions.hpp"
#include "../UI/UIManagement.hpp"

using namespace std;

CfwSelectPage::CfwSelectPage(vector<string> &folders)
{
	Folders = folders;
	if (folders.size() == 0)
		Title = "Couldn't find any cfw folder. Make sure you have either the \"atmosphere\", \"reinx\" or \"sxos\" folder in the root of your sd card as some cfws don't create it automatically. The folder name must be lowercase and without spaces.\nif your cfw isn't supported open an issue on Github.\nPress + to quit";
	else 
		Title = "Multiple cfw folders were detected, which one do you want to use ?";

}

CfwSelectPage::~CfwSelectPage()
{

}

static const int BtnWidth = 500;
static const int XCursorBtn = SCR_W / 2 - XCursorBtn / 2;

void CfwSelectPage::Render(int X, int Y)
{
	Utils::ImGuiSetupWin("CfwSelectPage", 10, 10);
	ImGui::SetWindowSize({SCR_W - 20, SCR_H - 20});
	ImGui::SetWindowFocus();

	ImGui::PushFont(font40);
	ImGui::TextWrapped(Title.c_str());
	ImGui::PopFont();

	ImGui::PushFont(font30);
	ImGui::SetCursorPos({ (float)XCursorBtn, ImGui::GetCursorPosY() + 30 });

	int count = 0;
	for (const auto &e : Folders)
	{
		ImGui::SetCursorPosX((float)XCursorBtn);
		if (ImGui::Button(e.c_str(), { BtnWidth, 50 }))
		{
			CfwFolder = e;
			PopPage();
		}
		count++;
	}

	ImGui::PopFont();
	Utils::ImGuiSetWindowScrollable();
	Utils::ImGuiCloseWin();
}

void CfwSelectPage::Update()
{
	if (gamepad.buttons[GLFW_GAMEPAD_BUTTON_START])
		SetAppShouldClose();
}




