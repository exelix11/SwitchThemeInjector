#pragma once
#include "../../SwitchThemesCommon/MyTypes.h"
#include "../../SwitchThemesCommon/SarcLib/Sarc.hpp"
#include "../../UI/UI.hpp"
#include <memory>

class ThemeEntry 
{
	public:
		static void DisplayInstallDialog(const std::string& path);

		enum class UserAction 
		{
			None,
			Enter,
			Preview
		};

		static std::unique_ptr<ThemeEntry> FromFile(const std::string& fileName);
		static std::unique_ptr<ThemeEntry> FromSZS(const std::vector<u8>& RawData);

		virtual ~ThemeEntry();
		
		static constexpr int EntryW = 860;

		virtual bool IsFolder() = 0;
		virtual bool CanInstall() = 0;		
		bool Install(bool ShowDialogs = true);
		virtual bool HasPreview() = 0;

		bool IsHighlighted();
		std::string GetPath() {return FileName;}
		
		virtual UserAction Render(bool OverrideColor = false);

		std::string InstallLog;
	protected:
		virtual bool DoInstall(bool ShowDialogs = true) = 0;
		virtual LoadedImage GetPreview() = 0;

		void AppendInstallMessage(const std::string& msg)
		{
			if (InstallLog.empty())
				InstallLog = msg;
			else
				InstallLog += "\n" + msg;
		}

		std::vector<u8> file;		
		
		std::string FileName;
		std::string lblFname;
		std::string lblLine1;
		std::string lblLine2;

		std::string CannotInstallReason;

		//Used to return by reference for the background image
		const static std::vector<u8> _emtptyVec;
};