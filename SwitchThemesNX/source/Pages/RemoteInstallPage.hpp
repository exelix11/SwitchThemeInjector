#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../UI/UI.hpp"
#include "../fs.hpp"
#include "ThemeEntry/ThemeEntry.hpp"
#include "RemoteInstall/RemoteInstall.hpp"
#include "../SwitchTools/InjectorInstall.hpp"

class RemoteInstallPage : public IPage
{
	public:
		RemoteInstallPage();	
		~RemoteInstallPage();
		
		void Render(int X, int Y) override;
		void Update() override;
	private:
		void DialogError(const std::string &msg);
		
		// Download install
		int ProviderIndex = 0;
		bool SelectedProviderStatic = false;
		const RemoteInstall::Provider& SelectedProvider();
		std::string RemoteInstallCode;
		std::string RemoteInstallBtnText;
		void SetRemoteInstallCode(const char* input);
		void StartRemoteInstallByCode();
		void StartRemoteInstallFixed(RemoteInstall::FixedTypes type);
		
		// Injector install
		InjectorInstall::Server server;
		void StartServer();
		void StopServer();
		void UpdateServer();
		std::unique_ptr<ThemeEntry> RemoteInstallFile = 0;
		std::string BtnStart;
		bool AutoInstall = false;
		
		// Layout stuff
		void CurItemBlockLeft();
		bool AllowLeft = true;
};