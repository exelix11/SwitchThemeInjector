#pragma once
#include <string>
#include <vector>
#include "../../SwitchThemesCommon/MyTypes.h"
#include "RemoteInstall.hpp"

namespace RemoteInstall::API
{
	struct Entry 
	{
		std::string Name;
		std::string Target;
		std::string Url;
		std::string Preview;
	};

	struct APIResponse 
	{
		std::vector<Entry> Entries;
		std::string GroupName;
	};

	std::string MakeUrl(const std::string& provider, const std::string& ID);

	APIResponse GetManifest(const std::string& Url);

	void ReloadProviders();
	const std::vector<Provider>& GetProviders();
	const Provider& GetProvider(size_t index);
	size_t ProviderCount();

	bool IsInitialized();
	void Initialize();
	void Finalize();

	/*
		API for remote install:
	
	- Base scheme
		The installer issues a GET request to the server, replacing the user-provided ID in the template URL, the server replies with OK status and a json response.
		In case the user provided ID refers to a single nxtheme file the response looks like this:
		{
		   "themes":[
		       {
		           "name":"Theme name",
		           "target":"home",
		           "url":"http://.../file.nxtheme",
		           "preview":"http://.../file.jpg",
		       }
		   ]
		}
		
		The name should be a short name describing the theme, layout info and author name are already part of the NXTheme file and not needed there.
		When saving on the sd card the installer will normalize and, if needed, shorten the name obtained from the NXTheme manifest.
	
		In case the user provided ID refers to multiple nxthemes file the response looks like this:
		{
		   "groupname" : "group name"
		   "themes":[
			   {
				   "name":"Theme name",
				   "target":"home",
				   "url":"http://.../file.nxtheme",
				   "preview":"http://.../file.jpg",
			   },
			   ... more themes with the same structure here
		   ]
		}

		The groupname field is used as the name for the folder that will contain the downloaded files.
		If the response doesn't contain a groupname a random one will be generated, to keep this user friendly always provide a group name.

	- Error handling
		Error messages from the server are not planned, the installer will display a generic error in case of any unexpected HTTP status code, network error, or malformed JSON.

	- Compatibility with graphql
		The specified json response can be inside of a "data" root element:
		{
			"data":{
				"themes" : [...]
			}
		}
		This is to support graphql responses from Themezer, the first website that implements this API	
	*/
}