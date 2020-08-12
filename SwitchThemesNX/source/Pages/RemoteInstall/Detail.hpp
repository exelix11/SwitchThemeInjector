#pragma once
#include "../../UI/UI.hpp"
#include "API.hpp"
#include <string>

namespace RemoteInstall 
{
	class DetailPage : public IUIControlObj
	{
	public:
		DetailPage(const RemoteInstall::API::Entry& entry, LoadedImage img);

		const RemoteInstall::API::Entry entry;

		void Update();
		void Render(int X, int Y);
		~DetailPage();
	private:
		std::string PartName;
		LoadedImage img;

		enum class Action : int 
		{
			Download = 1,
			Install = 2,
			DownloadInstall = 3
		};

		void UserDownload(Action act);

		std::vector<u8> DownloadData();
		std::vector<u8> DownloadedTheme;
	};
}