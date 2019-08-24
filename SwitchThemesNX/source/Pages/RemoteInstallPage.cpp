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

using namespace std;

RemoteInstallPage::~RemoteInstallPage()
{
	StopSocketing();
}

RemoteInstallPage::RemoteInstallPage() : 
BtnStart("Start remote install"),
lblInfo("You can install a theme directly from your pc with the theme injector, go to the 'Nxtheme builder' tab and click on 'Remote install...'"),
lblConfirm("Press A to install, B to cancel.")
{
	Name = "Remote Install";
}

void RemoteInstallPage::Render(int X, int Y)
{
	Utils::ImGuiSetupPage(this, X, Y);
	ImGui::PushFont(font30);
	if (entry)
	{
		entry->Render();
		ImGui::TextWrapped(lblConfirm.c_str());
	}
	else 
	{
		ImGui::TextWrapped(lblInfo.c_str());
		if (ImGui::Button(BtnStart.c_str()))
		{
			if (sock >= 0)
				StopSocketing();
			else
				StartSocketing();
		}
		PAGE_RESET_FOCUS;
		ImGui::TextWrapped("Keep the menu focus on this page or requests won't be executed");
	}
	ImGui::PopFont();
	Utils::ImGuiCloseWin();
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
	const char* hostname = "F:\remoteFile.bin";
#endif
	BtnStart = ("IP: " + string(hostname) + " - Press to stop");
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
	BtnStart = ("Start remote install");
}

void RemoteInstallPage::DialogError(const std::string &msg)
{
	Dialog("There was an error, try again.\n" + msg);
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
			ThemeSize = *reinterpret_cast<int*>(buf + 8);
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
				entry = new ThemeEntry(data);
				StopSocketing();
			}
		}
		return;
	}
#else
	if (filesystem::exists("F:/RemoteFile.bin"))
	{
		data = OpenFile("F:/RemoteFile.bin");
		ThemeSize = data.size();
		entry = new ThemeEntry(data);
		StopSocketing();
	}
#endif
}

void RemoteInstallPage::Update()
{
	if (entry)
	{
		if (KeyPressed(GLFW_GAMEPAD_BUTTON_A))
		{
			string overrideStr = "";
			if (gamepad.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER])//TODO shuffle
				overrideStr = MakeThemeShuffleDir();
			entry->InstallTheme(true,overrideStr);
			delete entry;
			entry = 0;
			StopSocketing();
			return;
		}
		else if (KeyPressed(GLFW_GAMEPAD_BUTTON_B))
		{
			delete entry;
			entry = 0;			
			StopSocketing();
			return;			
		}
	}
	
	if (Utils::PageLeaveFocusInput()){
		Parent->PageLeaveFocus(this);
		return;
	}
	
	if (entry) return;
	
	if (sock >= 0)
		SocketUpdate();
}