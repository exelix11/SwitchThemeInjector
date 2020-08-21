#pragma once
#include <vector>
#include <string>
#include "../SwitchThemesCommon/MyTypes.h"

namespace InjectorInstall{
	class Server
	{
	public:
		~Server() { StopHosting(); }

		void StartHosting();
		void StopHosting();
		void Clear();

		bool IsHosting() { return hostSock > 0; }
		void HostUpdate();
		std::string GetHostname();

		const std::vector<u8>& Buffer();
		bool HasFinished() { return Finished; }
	private:
		bool Finished = false;
		std::vector<u8> Data;

		u32 PayloadSize = 0;
		int hostSock = -1;
		int clientSock = -1;
	};
}