#include "DialogPages.hpp"
#include "../ViewFunctions.hpp"

using namespace std;

LoadingOverlay::LoadingOverlay(const string &msg) : text(msg) {}

void LoadingOverlay::Render(int X, int Y)
{		
	Utils::ImGuiSetupWin("Loading", 20, 20, ImGuiWindowFlags_NoDecoration & ~ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
	ImGui::SetWindowFocus();
	ImGui::SetWindowSize({ SCR_W - 30, SCR_H - 30});
	ImGui::PushFont(font30);
	ImGui::SetCursorPosY(400);
	Utils::ImGuiCenterString(text.c_str());
	ImGui::PopFont();
	Utils::ImGuiCloseWin();
}

void LoadingOverlay::Update() {}

DialogPage::DialogPage(const string &msg,const string &buttonMsg) : text(msg), btn(buttonMsg){}
DialogPage::DialogPage(const string &msg) : DialogPage(msg, "Continue") {}

void DialogPage::Render(int X, int Y)
{	
	Utils::ImGuiSetupWin("DialogPage", 20, 20, ImGuiWindowFlags_NoDecoration & ~ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
	ImGui::SetWindowFocus();
	ImGui::SetWindowSize({ SCR_W - 30, SCR_H - 30});
	ImGui::PushFont(font30);
	ImGui::SetCursorPos({ 10, 15 });
	ImGui::TextWrapped(text.c_str());
	
	if (ImGui::Button("   OK   "))
		PopPage(); 
	Utils::ImGuiSelectItemOnce();

	ImGui::PopFont();
	Utils::ImGuiSetWindowScrollable();
	Utils::ImGuiCloseWin();
}

void DialogPage::Update()
{

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
	ImGui::TextWrapped(text.c_str());

	if (ImGui::Button("   YES   "))
	{
		*result = true;
		PopPage();
	}
	Utils::ImGuiSelectItemOnce();
	ImGui::SameLine();
	if (ImGui::Button("   NO   "))
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

