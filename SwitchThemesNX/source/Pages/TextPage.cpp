#include "TextPage.hpp"
#include "../Version.hpp"
#include "../ViewFunctions.hpp"
#include "../UI/imgui/imgui_internal.h"

using namespace std;

TextPage::TextPage(const std::string& title, const std::string& text) :
	Text(text)
{
	Name = title;
	c_str = Text.c_str();
}

TextPage::TextPage(const char* title, const char* text) 
{
	Name = title;
	c_str = text;
}

void TextPage::Render(int X, int Y)
{
	Utils::ImGuiSetupPage(this, X, Y);
	ImGui::PushFont(font25);
	ImGui::TextWrapped(c_str);
	ImGui::PopFont();
	Utils::ImGuiSetWindowScrollable();
	Utils::ImGuiCloseWin();
}

void TextPage::Update()
{
	Parent->PageLeaveFocus(this);
}

CreditsPage::CreditsPage() :
	creditsText("NXThemes installer by exelix - " + Version::Name + " - Core Ver." + SwitchThemesCommon::CoreVer +
		'\n' + Version::Commit +
		"\nSource: github.com/exelix11/SwitchThemeInjector"+
		"\nDonations: ko-fi.com/exelix11\n\n")
{
	Name = "Credits";
}

extern void ShowFirstTimeHelp(bool WelcomeScr); //from main.cpp
void CreditsPage::Render(int X, int Y)
{
	Utils::ImGuiSetupPage(this, X, Y);
	ImGui::SetCursorPosY(20);
	ImGui::PushFont(font30);
	ImGui::TextWrapped(creditsText.c_str());
	ImGui::PopFont();

	ImGui::PushFont(font25);
	ImGui::TextWrapped(
		"Thanks to:\n"
		"Syroot for BinaryData lib\n"
		"AboodXD for Bntx editor and sarc lib\n"
		"shchmue for Lockpick\n"
		"SciresM for hactool\n"
		"Everyone from Atmosphere and libnx\n"
		"switch-stuff on github for the font converter\n"
		"Fincs for the hybrid_app template\n"
		"Everyone from the DearImgui github repo"
	);

	if (ImGui::Button("Show first startup info"))
		PushFunction([]() {ShowFirstTimeHelp(false); });
	PAGE_RESET_FOCUS;

	ImGui::PopFont();
	Utils::ImGuiSetWindowScrollable();
	Utils::ImGuiCloseWin();
}

void CreditsPage::Update()
{
	if (Utils::PageLeaveFocusInput())
		Parent->PageLeaveFocus(this);
}

