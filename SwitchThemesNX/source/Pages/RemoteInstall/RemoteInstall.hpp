#pragma once
#include <string>
#include <vector>

namespace RemoteInstall 
{
	struct Provider 
	{
		std::string Name;
		std::string UrlTemplate;
	};

	enum class FixedTypes
	{
		Random,
		Recent
	};

	void Initialize();
	void Finalize();
	bool IsInitialized();

	void Begin(const RemoteInstall::Provider& provider, const std::string& ID);
	void BeginType(const RemoteInstall::Provider& provider, FixedTypes type);
	void BeginRandom(const RemoteInstall::Provider& provider);
	void BeginRecent(const RemoteInstall::Provider& provider);

	const std::vector<Provider>& GetProviders();
}