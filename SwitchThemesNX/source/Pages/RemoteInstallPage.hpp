#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../UI/UI.hpp"
#include "../fs.hpp"
#include "ThemeEntry/ThemeEntry.hpp"
#ifdef __SWITCH__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#endif
#include "RemoteInstall/RemoteInstall.hpp"

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
		
		std::string RemoteInstallCode;
		std::string RemoteInstallBtnText;
		void SetRemoteInstallCode(const char* input);
		void StartRemoteInstall(const RemoteInstall::Provider& provider);

		int sock = -1;	
		//For SocketUpdate:
		int curSock = -1;
		u32 ThemeSize = 0;
		std::vector<u8> data;
		
		std::unique_ptr<ThemeEntry> entry = 0;
		std::string BtnStart;

		bool AutoInstall = false;
		
		void CurItemBlockLeft();
		bool AllowLeft = true;
};