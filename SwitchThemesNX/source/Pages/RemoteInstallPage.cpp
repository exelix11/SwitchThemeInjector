#include "RemoteInstallPage.hpp"
#include "../ViewFunctions.hpp"
#ifdef __SWITCH__
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#endif
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
	StopSocketing();
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
		if (ImGui::Button("Search###SearchBtn", {150, 0}) && RemoteInstallCode != "")
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

	if (entry)
	{
		entry->Render();
		ImGui::TextWrapped("Press A to install, B to cancel");
	}
	else 
	{
		ImGui::TextWrapped("You can install a theme directly from your pc with the theme injector, go to the 'NXTheme builder' tab and click on 'Remote install...'");
		if (ImGui::Button(BtnStart.c_str()))
		{
			if (sock >= 0)
				StopSocketing();
			else
				StartSocketing();
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
	if (entry)
	{
		if (KeyPressed(GLFW_GAMEPAD_BUTTON_A) || AutoInstall)
		{
			entry->Install(!AutoInstall);
			entry = nullptr;
			StopSocketing();

			if (AutoInstall)
			{
				if (PayloadReboot::Init())
					PayloadReboot::Reboot();
				else
					Dialog("Couldn't initialize reboot to payload !");
			}

			return;
		}
		else if (KeyPressed(GLFW_GAMEPAD_BUTTON_B))
		{
			entry = nullptr;
			StopSocketing();
			return;
		}
	}

	if (Utils::PageLeaveFocusInput(AllowLeft)) {
		Parent->PageLeaveFocus(this);
		return;
	}

	if (entry) return;

	if (sock >= 0)
		SocketUpdate();
}

void RemoteInstallPage::SetRemoteInstallCode(const char* input)
{
	RemoteInstallCode = std::string(input);
	if (RemoteInstallCode == "")
		RemoteInstallBtnText = "Input text###themeIDinput";
	else
		RemoteInstallBtnText = RemoteInstallCode + "###themeIDinput";
}

void RemoteInstallPage::StartSocketing()
{
#if __SWITCH__
	if (sock != -1)
		return;
	
	int err;
	struct sockaddr_in temp;
	
	sock=socket(AF_INET,SOCK_STREAM,0);
	if (sock < 0)
	{
		Dialog("Couldn't start socketing (socket error)");
		sock = -1;
		return;
	}
	temp.sin_family=AF_INET;
	temp.sin_addr.s_addr=INADDR_ANY;
	temp.sin_port=htons(5000);
	
	err=fcntl(sock,F_SETFL,O_NONBLOCK);
	const int optVal = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*) &optVal, sizeof(optVal));
	if (err)
	{
		Dialog("Couldn't start socketing (fcntl error)");
		StopSocketing();
		return;
	}
	
	err=bind(sock,(struct sockaddr*) &temp,sizeof(temp));
	if (err)
	{
		Dialog("Couldn't start socketing (bind error)");
		StopSocketing();
		return;
	}
	
	err=listen(sock,1);
	if (err)
	{
		Dialog("Couldn't start socketing (listen error)");
		StopSocketing();
		return;
	}
	
	char hostname[128];
	err = gethostname(hostname, sizeof(hostname));
	if(err != 0)
	{
		Dialog("Couldn't start socketing (gethostname error)");
		StopSocketing();
		return;
	}
	
#else
	sock = 66;
	const char* hostname = "F:\\remoteFile.bin";
#endif
	BtnStart = ("IP: " + string(hostname) + " - Press to stop###InstallBtn");
}

void RemoteInstallPage::StopSocketing()
{
#if __SWITCH__
	if (curSock != -1)
		close(curSock);
	if (sock != -1)
		close(sock);
#endif
	curSock = -1;
	ThemeSize = 0;
	sock = -1;
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

void RemoteInstallPage::SocketUpdate()
{	
	if (sock < 0) 
	{
		return;
	}	
#if __SWITCH__
	int size = -1;
	if (curSock == -1 && (curSock=accept(sock,0,0))!=-1)
	{
		u8 buf[12]; 
		memset(buf,0,sizeof(buf));
		if ((size=recv(curSock,buf,sizeof(buf),0)) < 0)
		{
			DialogError("(Couldn't read any data.)");
			StopSocketing();
			return;
		}
		else
		{			
			if (strncmp((char*)buf, "theme", 5) != 0)
			{
				DialogError("(Unexpected data received.)");
				StopSocketing();
				return;
			}
			memcpy(&ThemeSize, buf + 8, sizeof(u32));
			if (ThemeSize < 50 || ThemeSize > 2000000)
			{
				DialogError("(Invalid size: " + to_string(ThemeSize) + ")");
				StopSocketing();
				return;				
			}
			data.clear();
			data.reserve(ThemeSize);
		}		
	}
	if (ThemeSize && curSock != -1)
	{
		DisplayLoading("Loading...");
		u8 tmp[10];
		while ((size = recv(curSock,tmp,10,0)) > 0)
		{
			for (int i = 0; i < size; i++)
				data.push_back(tmp[i]);
		}
		if (data.size() == ThemeSize || size == 0 || (size == -1 && errno != EWOULDBLOCK)){
			if (data.size() != ThemeSize)
				DialogError("(Unexpected data count: " + to_string(size) + ")");
			else
			{
				write(curSock,"ok",2);
				entry = ThemeEntry::FromSZS(data);
				StopSocketing();
			}
		}
		return;
	}
#else
	if (filesystem::exists("F:/RemoteFile.bin"))
	{
		data = fs::OpenFile("F:/RemoteFile.bin");
		ThemeSize = data.size();
		entry = ThemeEntry::FromSZS(data);
		StopSocketing();
	}
#endif
}