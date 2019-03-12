#pragma once
#include "../SwitchThemesCommon/MyTypes.h"
#include "../SwitchThemesCommon/SarcLib/Sarc.hpp"
#include "../UI/UI.hpp"

class ThemeEntry 
{
	public:
		ThemeEntry(const std::string &fileName);
		ThemeEntry(const std::vector<u8> &RawData);
		~ThemeEntry();
		
		bool IsFolder = false;
		bool CanInstall = true;		
		bool InstallTheme(bool ShowDialogs = true, const std::string &homeDirOverride = "");
		
		std::string GetPath() {return FileName;}
		bool HasPreview() {return NXThemeHasPreview;}
		
		bool Highlighted = false;
		void Render(int X, int Y, bool selected);
		SDL_Rect GetRect();
	private:
		bool LegacyTheme();
		bool IsFont();
	
		void ParseTheme();
		void ParseLegacyTheme();
		void ParseNxTheme();
		void ParseFont();		
		void NXLoadPreview();
		
		std::vector<u8> DecompressedFile;
		std::vector<u8> file;		
		SARC::SarcData SData;
	
		std::string FileName;
		SDL_Rect Size;
		Label lblFname;
		Label lblLine1;
		Label lblLine2;
		
		bool NXThemeHasPreview = false;
		Image *Preview = 0;
};