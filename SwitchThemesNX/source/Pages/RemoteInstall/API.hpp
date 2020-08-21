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

		// At least one of those must be a valid URL
		const std::string& PreviewUrl() const { return _Preview == "" ? _Thumbnail : _Preview; }
		const std::string& ThumbUrl() const { return _Thumbnail == "" ? _Preview : _Thumbnail; }

		// Accessing these directly shouldn't be needed
		std::string _Preview;
		std::string _Thumbnail;
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
	
	- URLs and Querys
		The remote servers also called providers offer an url formatted like this: http://example.provider/q?id=%%ID%%
		The %%ID%% part is replaced with the text the user inputs in the download screen.
		The format for the user input is not specified, how it's used is up the server.
		> In themezer it's used first as an ID lookup to find specific themes and then as a search query.
		
		There are special queries that the installer issues automatically and those are:
		`__special_random` and `__special_recent` which are used to retrieve random and recent themes from the provider.
		In general strings starting with `__special` should be considered reserved for future features even though it's unlikely.
		When defining a custom provider these can be disabled by setting `Static` to true

	- Base scheme
		The installer issues a GET request to the server, the server replies with OK status and a json response.
		In case the user provided ID refers to a single nxtheme file the response looks like this:
		{
		   "themes":[
		       {
		           "name":"Theme name",
		           "target":"home",
		           "url":"http://.../file.nxtheme",
		           "preview":"http://.../file.jpg",
				   "thumbnail":"http://.../file.jpg"
		       }
		   ]
		}
		
		The name should be a short name describing the theme, layout info and author name are already part of the NXTheme file and not needed there.
		When saving on the sd card the installer will normalize and, if needed, shorten the name obtained from the NXTheme manifest.
	
		The entry must have at least one preview image between `preview` and `thumbnail`, having both is ideal but not needed.
		`preview` is downloaded for full screen previewing, `thumbnail` for lists.
		If just `preview` is present loading many themes at the same time could be slow, having just `thumbnail` would scale baldly for full screen previews.
		The images don't have a set size but it makes no sense to have files bigger than 1280x720

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