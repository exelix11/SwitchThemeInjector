#include "RemoteInstallPage.hpp"
#include "../ViewFunctions.hpp"
#include "../SwitchTools/PayloadReboot.hpp"
#include <numeric>
#include "RemoteInstall/API.hpp"
#include "../SwitchThemesCommon/Layouts/json.hpp"

using namespace std;

static bool ComboBoxApiProviderGetter(void*, int index, const char** str)
{
	if (index < 0 || (size_t)index >= RemoteInstall::API::ProviderCount())
		return false;
	
	*str = RemoteInstall::API::GetProvider(index).Name.c_str();
	return true;
}

RemoteInstallPage::~RemoteInstallPage()
{
	if (!UseLowMemory)
		RemoteInstall::Finalize();
}

RemoteInstallPage::RemoteInstallPage() : 
BtnStart("Start remote install###InstallBtn")
{
	Name = "Download themes";
	if (!UseLowMemory)
	{
		SetRemoteInstallCode("");
		RemoteInstall::Initialize();
	}
}

void RemoteInstallPage::Render(int X, int Y)
{
	AllowLeft = true;

	Utils::ImGuiSetupPage(this, X, Y, DefaultWinFlags & ~ImGuiWindowFlags_NoScrollbar);
	
	if (!RemoteInstallFile)
	{
		ImGui::PushFont(font40);
		ImGui::Text("Download from the internet");
		ImGui::PopFont();

		if (UseLowMemory)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
			ImGui::TextWrapped("This feature is not available while running in applet mode, launch with title takeover.");
			ImGui::PopStyleColor();
		}
		else
		{
			ImGui::TextWrapped("Select a provider from the list to download themes.\nYou can add custom providers by editing the TBD file on your sd card."); //TODO
			ImGui::PushItemWidth(500);
			ImGui::Combo("###ProviderSelection", &ProviderIndex, ComboBoxApiProviderGetter, nullptr, RemoteInstall::API::ProviderCount());
			PAGE_RESET_FOCUS;
			if (ImGui::IsItemFocused())
				ImGui::SetScrollY(0);

			ImGui::SameLine();
			if (ImGui::Button("Random themes"))
				StartRemoteInstallFixed(RemoteInstall::FixedTypes::Random);
			CurItemBlockLeft();
			ImGui::SameLine();
			if (ImGui::Button("New themes"))
				StartRemoteInstallFixed(RemoteInstall::FixedTypes::Recent);
			CurItemBlockLeft();

			ImGui::TextWrapped("Or search a theme by ID");
			ImGui::PushStyleColor(ImGuiCol_Button, 0xDFDFDFDF);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, 0xEFEFEFEF);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, 0xFFFFFFFF);
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
			if (ImGui::Button(RemoteInstallBtnText.c_str(), { 300, 0 }))
				SetRemoteInstallCode(PlatformTextInput(RemoteInstallCode.c_str()));
			ImGui::PopStyleColor(4);
			ImGui::SameLine(0, 20);
			if (ImGui::Button("Search###SearchBtn", { 150, 0 }) && RemoteInstallCode != "")
				StartRemoteInstallByCode();
			CurItemBlockLeft();
			ImGui::TextWrapped("IDs are not names, searching a theme by name won't work, open your provider in the browser, select a theme and it should show its unique ID.");
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::PushFont(font40);
		ImGui::Text("Remote install from the theme injector");
		ImGui::PopFont();
	}

	if (RemoteInstallFile)
	{
		if (RemoteInstallFile->Render() == ThemeEntry::UserAction::Enter || AutoInstall)
			PushFunction([this]() {
				RemoteInstallFile->Install(!AutoInstall);
				RemoteInstallFile = nullptr;

				if (AutoInstall)
				{
					if (PayloadReboot::Init())
						PayloadReboot::Reboot();
					else
						Dialog("Couldn't initialize reboot to payload !");
				}
			});
		if (ImGui::IsWindowFocused())
			Utils::ImGuiSelectItem();

		ImGui::TextWrapped("Press A to install, B to cancel");
	}
	else 
	{
		ImGui::TextWrapped("You can install a theme directly from your pc with the theme injector, go to the 'NXTheme builder' tab and click on 'Remote install...'");
		if (ImGui::Button(BtnStart.c_str()))
		{
			if (!server.IsHosting())
				StartServer();
			else
				StopServer();
		}
		if (UseLowMemory) PAGE_RESET_FOCUS;
		ImGui::TextWrapped("Keep the menu focus on this page or requests won't be executed");
		ImGui::Checkbox("Automatically install and reboot", &AutoInstall);
	}
	Utils::ImGuiSetWindowScrollable();

	Utils::ImGuiCloseWin();
}

void RemoteInstallPage::StartRemoteInstallByCode()
{
	PushFunction([this]() {
		try {
			DisplayLoading("Loading...");
			RemoteInstall::Begin(SelectedProvider(), RemoteInstallCode);
		}
		catch (nlohmann::json::type_error& ex)	{
			DialogBlocking("There was an error parsing the response from the server, this often mean that the code you requested could not be found, make sure that the code is valid.\n\nError message:\n"s + ex.what());
		}
		catch (std::exception& ex) {
			DialogBlocking("There was an error processing the request, make sure that the code is valid and that you are connected to the internet.\n\nError message:\n"s + ex.what());
		}
	});
}

void RemoteInstallPage::StartRemoteInstallFixed(RemoteInstall::FixedTypes type)
{
	PushFunction([this, type]() {
		try {
			DisplayLoading("Loading...");
			RemoteInstall::BeginType(SelectedProvider(), type);
		}
		catch (std::exception& ex) {
			DialogBlocking("There was an error processing the request, make sure you are connected to the internet and try again in a bit, if it still doesn't work it's possible that the selected provider doesn't support this option.\n\nError message:\n"s + ex.what());
		}
	});
}

void RemoteInstallPage::CurItemBlockLeft() 
{
	AllowLeft &= !ImGui::IsItemFocused();
}

void RemoteInstallPage::Update()
{
	if (RemoteInstallFile && KeyPressed(GLFW_GAMEPAD_BUTTON_B))
	{
		RemoteInstallFile = nullptr;
		return;
	}

	if (Utils::PageLeaveFocusInput(AllowLeft)) {
		Parent->PageLeaveFocus(this);
		return;
	}

	if (RemoteInstallFile) return;

	UpdateServer();
}

void RemoteInstallPage::SetRemoteInstallCode(const char* input)
{
	RemoteInstallCode = std::string(input);
	if (RemoteInstallCode == "")
		RemoteInstallBtnText = "Input text###themeIDinput";
	else
		RemoteInstallBtnText = RemoteInstallCode + "###themeIDinput";
}

void RemoteInstallPage::StartServer()
{
	try 
	{
		server.StartHosting();
		BtnStart = ("IP: " + server.GetHostname() + " - Press to stop###InstallBtn");
	}
	catch (std::exception& ex)
	{
		Dialog(ex.what());
	}
}

void RemoteInstallPage::StopServer()
{
	server.StopHosting();
	BtnStart = "Start remote install###InstallBtn";
}

void RemoteInstallPage::DialogError(const std::string &msg)
{
	Dialog("There was an error, try again.\n" + msg);
}

const RemoteInstall::Provider& RemoteInstallPage::SelectedProvider()
{
	return RemoteInstall::API::GetProvider(ProviderIndex);
}

void RemoteInstallPage::UpdateServer()
{	
	try 
	{
		server.HostUpdate();
		if (server.IsFinished())
		{
			RemoteInstallFile = ThemeEntry::FromSZS(server.Buffer());
			server.Clear();
			StopServer();
		}
	}
	catch (std::exception& ex)
	{
		Dialog(ex.what());
	}
}