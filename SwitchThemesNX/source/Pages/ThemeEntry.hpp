#pragma once
#include "../SwitchThemesCommon/MyTypes.h"
#include "../SwitchThemesCommon/SarcLib/Sarc.hpp"
#include "../UI/UI.hpp"

class ThemeEntry 
{
	public:
		ThemeEntry(const std::string &fileName);
		
		bool IsFolder = false;
		bool CanInstall = true;
		void InstallTheme();
		
		std::string GetPath() {return FileName;}
		
		void Render(int X, int Y, bool selected);
		SDL_Rect GetRect();
	private:
		bool LegacyTheme();
	
		void ParseTheme();
		void ParseLegacyTheme();
		void ParseNxTheme();
		
		std::vector<u8> DecompressedFile;
		std::vector<u8> file;		
		SARC::SarcData SData;
	
		std::string FileName;
		SDL_Rect Size;
		Label lblFname;
		Label lblLine1;
		Label lblLine2;
};