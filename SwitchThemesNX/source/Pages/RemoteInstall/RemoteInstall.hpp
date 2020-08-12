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

	void Initialize();
	void Finalize();
	bool IsInitialized();

	void Begin(const std::string& provider, const std::string& ID);

	const std::vector<Provider>& GetProviders();
}