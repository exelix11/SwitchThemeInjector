#include "RemoteInstallPage.hpp"
#include "../input.hpp"
#include "../ViewFunctions.hpp"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

using namespace std;

RemoteInstallPage::~RemoteInstallPage()
{
	StopSocketing();
}

RemoteInstallPage::RemoteInstallPage() : 
BtnStart("Start remote install"),
lblInfo("You can install a theme directly from your pc with the theme injector, go to the 'Nxtheme builder' tab and click on 'Remote install...'", WHITE, 890, font30),
lblConfirm("Press A to install, B to cancel.",WHITE, 890, font30)
{
	Name = "Remote Install";
	BtnStart.selected = false;
}

void RemoteInstallPage::Render(int X, int Y)
{
	if (entry)
	{
		entry->Render(X + 10, Y + 10,true);
		lblConfirm.Render(X + 10, Y + 100);
	}
	else 
	{
		lblInfo.Render(X + 10, Y + 20);
		BtnStart.Render(X + 10, Y + 20 + lblInfo.GetSize().h + 10);
	}
}

void RemoteInstallPage::StartSocketing()
{
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
	
	BtnStart.SetString("IP: " + string(hostname) + " - Press to stop");
}

void RemoteInstallPage::StopSocketing()
{
	if (curSock != -1)
		close(curSock);
	curSock = -1;
	ThemeSize = 0;
	if (sock != -1)
		close(sock);
	sock = -1;
	BtnStart.SetString("Start remote install");
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
			if (strncmp(buf, "theme", 5) != 0)
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
}

void RemoteInstallPage::Update()
{
	if (entry)
	{
		if (kDown & KEY_A)
		{
			string overrideStr = "";
			if (kHeld & KEY_R)
				overrideStr = MakeThemeShuffleDir();
			entry->InstallTheme(true,overrideStr);
			delete entry;
			entry = 0;
			StopSocketing();
			return;
		}
		else if (kDown & KEY_B)
		{
			delete entry;
			entry = 0;			
			StopSocketing();
			return;			
		}
	}
	
	if (kDown & KEY_B || kDown & KEY_LEFT){
		BtnStart.selected = false;
		Parent->PageLeaveFocus(this);
		return;
	}
	
	BtnStart.selected = true;
	
	if (entry) return;
	if (sock >= 0)
	{
		SocketUpdate();
		if (kDown & KEY_A)
		{
			StopSocketing();
			return;
		}
	}
	else 
	{
		if (kDown & KEY_A)
		{
			StartSocketing();
			return;
		}
	}	
}