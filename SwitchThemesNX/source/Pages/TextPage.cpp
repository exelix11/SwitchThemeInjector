#include "TextPage.hpp"
#include "../ViewFunctions.hpp"
#include "../UI/imgui/imgui_internal.h"

using namespace std;

TextPage::TextPage(const std::string& title, const std::string& text) :
	Text(text)
{
	Name = title;
}

void TextPage::Render(int X, int Y)
{
	Utils::ImGuiSetupPage("TextPage", X, Y, focused);
	ImGui::PushFont(font30);
	ImGui::TextWrapped(Text.c_str());
	ImGui::PopFont();
	Utils::ImGuiSetWindowScrollable();
	Utils::ImGuiCloseWin();
}

void TextPage::Update()
{
	Parent->PageLeaveFocus(this);
}


CreditsPage::CreditsPage() :
	creditsText("NXThemes installer by exelix\n" + VersionString + " - Core Ver." + SwitchThemesCommon::CoreVer +
		"\nhttps://github.com/exelix11/SwitchThemeInjector\n\n"),
	creditsText2(
		"Thanks to:\n"
		"Syroot for BinaryData lib\n"
		"AboodXD for Bntx editor and sarc lib\n"
		"shchmue for Lockpick\n"
		"ScriesM for hactool\n"
		"Everyone from Atmosphere and libnx\n"
		"switch-stuff on github for the font converter\n"
		"Fincs for the hybrid_app template\n"
		"Everyone from the DearImgui github repo")
{
	Name = "Credits";
}

extern void ShowFirstTimeHelp(bool WelcomeScr); //from main.cpp
void CreditsPage::Render(int X, int Y)
{
	Utils::ImGuiSetupPage("CreditsPage", X, Y, focused);
	ImGui::SetCursorPosY(20);
	ImGui::PushFont(font30);
	ImGui::TextWrapped(creditsText.c_str());
	ImGui::PopFont();

	ImGui::PushFont(font25);
	ImGui::TextWrapped(creditsText2.c_str());

	if (ImGui::Button("Show first startup info"))
		ShowFirstTimeHelp(false);
	PAGE_RESET_FOCUS
	ImGui::SameLine();
	if (ImGui::Button("Show licenses"))
	{
		auto f = OpenFile(ASSET("licenses.txt"));
		Dialog(string((char*)f.data(), f.size()));
	}

	IsLayoutBlockingLeft = GImGui->NavId == ImGui::GetCurrentWindow()->GetID("Show licenses");

	ImGui::PopFont();
	Utils::ImGuiSetWindowScrollable();
	Utils::ImGuiCloseWin();
}

void CreditsPage::Update()
{
	if (Utils::PageLeaveFocusInput(!IsLayoutBlockingLeft))
		Parent->PageLeaveFocus(this);
}

