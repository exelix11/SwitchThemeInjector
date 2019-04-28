#pragma once
#include <switch.h>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../UI/UI.hpp"
#include "../fs.hpp"
#include "ThemeEntry.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

class RemoteInstallPage : public IPage
{
	public:
		RemoteInstallPage();	
		~RemoteInstallPage();
		
		void Render(int X, int Y) override;
		void Update() override;
	private:
		void StartSocketing();
		void StopSocketing();
		void SocketUpdate();
		
		void DialogError(const std::string &msg);
		
		int sock = -1;
	
		//For SocketUpdate:
		int curSock = -1;
		int ThemeSize = 0;
		std::vector<u8> data;
		
		Label lblInfo;
		Label lblConfirm;
		ThemeEntry *entry = 0;
		Button BtnStart;
};