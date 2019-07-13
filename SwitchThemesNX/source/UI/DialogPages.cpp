#include "DialogPages.hpp"
#include "../ViewFunctions.hpp"

using namespace std;

LoadingOverlay::LoadingOverlay(const string &msg) : text(msg) {}

void LoadingOverlay::Render(int X, int Y)
{		
	Utils::ImGuiSetupWin("Loading", 20, 20);
	ImGui::SetWindowFocus();
	ImGui::SetWindowSize({ SCR_W - 30, SCR_H - 30});
	ImGui::PushFont(font30);
	ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2 - 20);
	Utils::ImGuiCenterString(text.c_str());
	ImGui::PopFont();
	Utils::ImGuiCloseWin();
}

void LoadingOverlay::Update() {}

DialogPage::DialogPage(const string &msg,const string &buttonMsg) : text(msg), btn(buttonMsg){}
DialogPage::DialogPage(const string &msg) : DialogPage(msg, "Continue") {}

void DialogPage::Render(int X, int Y)
{	
	Utils::ImGuiSetupWin("DialogPage", 20, 20, DefaultWinFlags | ImGuiWindowFlags_NoBringToFrontOnFocus);
	ImGui::SetWindowSize({ SCR_W - 40, SCR_H - 40});
	ImGui::PushFont(font30);
	ImGui::SetCursorPos({ 10, 15 });

	Utils::ImGuiSetupWin("DialogContent", 20, 20, DefaultWinFlags & ~ImGuiWindowFlags_NoScrollbar);
	ImGui::SetWindowFocus();
	ImGui::SetWindowSize({ SCR_W - 40 ,  SCR_H - 95 });
	ImGui::PushTextWrapPos(0.0f);
	ImGui::TextUnformatted(text.c_str());
	ImGui::PopTextWrapPos();
	Utils::ImGuiSetWindowScrollable();
	Utils::ImGuiCloseWin();

	ImGui::SetCursorPosY(SCR_H - 90);
	if (Utils::ImGuiCenterButtons({"   OK   "}) == 0)
		PopPage();
	Utils::ImGuiSelectItemOnce();

	ImGui::PopFont();
	Utils::ImGuiCloseWin();
}

void DialogPage::Update()
{
	if (KeyPressed(GLFW_GAMEPAD_BUTTON_A))
		PopPage();
}

YesNoPage::YesNoPage(const string &msg, bool *outRes): text(msg) 
{
	result = outRes;
}

void YesNoPage::Render(int X, int Y)
{
	Utils::ImGuiSetupWin("YesNoPage", 20, 20, ImGuiWindowFlags_NoDecoration & ~ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
	ImGui::SetWindowFocus();
	ImGui::SetWindowSize({ SCR_W - 30, SCR_H - 30 });
	ImGui::PushFont(font30);
	ImGui::SetCursorPosY(30);
	ImGui::PushTextWrapPos(0.0f);
	ImGui::TextUnformatted(text.c_str());
	ImGui::PopTextWrapPos();

	int res = Utils::ImGuiCenterButtons({ "      YES      ", "      NO      " });

	if (res == 0)
	{
		*result = true;
		PopPage();
	}
	else if (res == 1)
	{
		*result = false;
		PopPage();
	}
	
	ImGui::PopFont();
	Utils::ImGuiSetWindowScrollable();
	Utils::ImGuiCloseWin();
}

void YesNoPage::Update()
{

}

bool YesNoPage::Ask(const std::string &msg)
{
	bool result = false;
	PushPageBlocking(new YesNoPage(msg, &result));
	return result;
}

