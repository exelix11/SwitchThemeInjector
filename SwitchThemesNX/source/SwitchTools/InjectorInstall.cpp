#include "InjectorInstall.hpp"
#include <stdexcept>
#include <cstring>

#if __SWITCH__
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#else
#include <filesystem>
#include "../fs.hpp"
#endif

void InjectorInstall::Server::StartHosting()
{
	if (IsHosting())
		throw std::runtime_error("The server has already been started");

	Clear();
#if __SWITCH__
	int err;
	struct sockaddr_in temp;

	hostSock = socket(AF_INET, SOCK_STREAM, 0);
	if (hostSock < 0)
	{
		hostSock = -1;
		throw std::runtime_error("Couldn't start socketing (socket error)");
	}
	temp.sin_family = AF_INET;
	temp.sin_addr.s_addr = INADDR_ANY;
	temp.sin_port = htons(5000);

	err = fcntl(hostSock, F_SETFL, O_NONBLOCK);
	const int optVal = 1;
	setsockopt(hostSock, SOL_SOCKET, SO_REUSEADDR, (const void*)&optVal, sizeof(optVal));
	if (err)
	{
		StopHosting();
		throw std::runtime_error("Couldn't start socketing (fcntl error)");
	}

	err = bind(hostSock, (struct sockaddr*)&temp, sizeof(temp));
	if (err)
	{
		StopHosting();
		throw std::runtime_error("Couldn't start socketing (bind error)");
	}

	err = listen(hostSock, 1);
	if (err)
	{
		StopHosting();
		throw std::runtime_error("Couldn't start socketing (listen error)");
	}
#else
	hostSock = 1;
#endif
}

void InjectorInstall::Server::StopHosting()
{
	if (!IsHosting())
		return;

#if __SWITCH__
	if (hostSock != -1)
		close(hostSock);
	if (clientSock != -1)
		close(clientSock);
#endif

	hostSock = -1;
	clientSock = 1;
	PayloadSize = 0;
}

void InjectorInstall::Server::Clear()
{
	Finished = false;
	Data.clear();
}

void InjectorInstall::Server::HostUpdate()
{
	if (!IsHosting())
		return;
#if __SWITCH__
	int size = -1;
	if (clientSock == -1 && (clientSock = accept(hostSock, 0, 0)) != -1)
	{
		u8 buf[12];
		memset(buf, 0, sizeof(buf));
		if ((size = recv(clientSock, (char*)buf, sizeof(buf), 0)) < 0)
		{
			StopHosting();
			throw std::runtime_error("Couldn't read any data.");			
		}
		else
		{
			if (strncmp((char*)buf, "theme", 5) != 0)
			{
				StopHosting();
				throw std::runtime_error("Unexpected data received.");
			}
			std::memcpy(&PayloadSize, buf + 8, sizeof(PayloadSize));
			if (PayloadSize < 50 || PayloadSize > 2000000)
			{
				StopHosting();
				throw std::runtime_error("Invalid size: " + std::to_string(PayloadSize));
			}
			Data.clear();
			Data.reserve(PayloadSize);
		}
	}

	if (PayloadSize && clientSock != -1)
	{
		u8 tmp[10];
		while ((size = recv(clientSock, (char*)tmp, 10, 0)) > 0)
		{
			for (int i = 0; i < size; i++)
				Data.push_back(tmp[i]);
		}
		if (Data.size() == PayloadSize || size == 0 || (size == -1 && errno != EWOULDBLOCK)) {
			if (Data.size() != PayloadSize)
				throw std::runtime_error("Unexpected data count: " + std::to_string(size));
			else
			{
				write(clientSock, "ok", 2);
				Finished = true;
				StopHosting();
			}
		}
		return;
	}
#else
	if (std::filesystem::exists("F:/RemoteFile.bin"))
	{
		Data = fs::OpenFile("F:/RemoteFile.bin");
		PayloadSize = Data.size();
		Finished = true;
		StopHosting();
	}
#endif
}

std::string InjectorInstall::Server::GetHostname()
{
#if __SWITCH__
	char hostname[128];
	int err = gethostname(hostname, sizeof(hostname));
	if (err != 0)
	{
		StopHosting();
		throw std::runtime_error("Couldn't start socketing (gethostname error)");
	}
	return hostname;
#else
	return "F:\\remoteFile.bin";
#endif
}

const std::vector<u8>& InjectorInstall::Server::Buffer()
{
	return Data;
}
