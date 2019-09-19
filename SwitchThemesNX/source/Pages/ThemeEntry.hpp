#pragma once
#include "../SwitchThemesCommon/MyTypes.h"
#include "../SwitchThemesCommon/SarcLib/Sarc.hpp"
#include "../UI/UI.hpp"

class ThemeEntry 
{
	public:
		enum class UserAction 
		{
			None,
			Install,
			Preview
		};

		ThemeEntry(const std::string &fileName);
		ThemeEntry(const std::vector<u8> &RawData);
		~ThemeEntry();
		
		static constexpr int EntryW = 860;

		bool IsFolder = false;
		bool CanInstall = true;		
		bool InstallTheme(bool ShowDialogs = true, const std::string &homeDirOverride = "");
		
		bool IsHighlighted();

		std::string GetPath() {return FileName;}
		bool HasPreview() {return NXThemeHasPreview;}
		
		UserAction Render(bool OverrideColor = false);
	private:
		bool LegacyTheme();
		bool IsFont();
	
		void ParseTheme();
		void ParseLegacyTheme();
		void ParseNxTheme();
		void ParseFont();		
		LoadedImage NXGetPreview();

		const std::vector<u8>& NxThemeGetBgImage();
		
		std::vector<u8> file;		
		SARC::SarcData SData;
	
		std::string FileName;
		std::string lblFname;
		std::string lblLine1;
		std::string lblLine2;
		
		bool NXThemeHasPreview = false;
		int NXThemeVer = -1;

		//Used to return by reference for the background image
		const static std::vector<u8> _emtptyVec;
};