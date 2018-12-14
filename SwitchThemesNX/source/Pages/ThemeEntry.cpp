#include "ThemeEntry.hpp"
#include "../SwitchThemesCommon/NXTheme.hpp"
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../ViewFunctions.hpp"
#include "../fs.hpp"

using namespace std;

const int EntryW = 860;
const int EntryH = 96;

ThemeEntry::ThemeEntry(const string &fileName) :
	lblFname("", WHITE, -1,font30),
	lblLine1("", WHITE, -1,font25),
	lblLine2("", WHITE, -1,font25),
	Size {0,0, EntryW, EntryH }
{
	FileName = fileName;
	ParseTheme();
}

bool ThemeEntry::LegacyTheme()
{
	return StrEndsWith(FileName,".szs");
}

void ThemeEntry::InstallTheme() 
{
	if (!CanInstall)
	{
		Dialog("Can't install this theme, check that it hasn't been corrupted and that you are using an updated version of this installer");
		return;
	}
	if (LegacyTheme())
	{
		DisplayLoading("Installing...");
		PatchTemplate patch = SwitchThemesCommon::DetectSarc(SData);
		CreateThemeStructure(patch.TitleId);
		WriteFile(CfwFolder + "/titles/" + patch.TitleId + "/romfs/lyt/" + patch.szsName, file);
	}
	else 
	{
		auto themeInfo = ParseNXThemeFile(SData);
		string BaseSzs = "/themes/systemData/" + ThemeTargetToFileName[themeInfo.Target];
		if (!filesystem::exists(BaseSzs))
		{
			Dialog(	"Can't install this theme because the original " + ThemeTargetToFileName[themeInfo.Target] + " is missing from systemData.\n"
					"To install theme packs (.nxtheme files) you need to dump the home menu romfs following the guide in the \"NCA extraction\" tab");
			return;
		}
		DisplayLoading("Installing...");
		auto BaseFile = OpenFile(BaseSzs);
		BaseFile = Yaz0::Decompress(BaseFile);
		auto ToPatch = SARC::Unpack(BaseFile);
		auto patch = SwitchThemesCommon::DetectSarc(ToPatch);
		if (patch.FirmName == "")
		{
			Dialog("Couldn't find any patch for " + BaseSzs + "\nThe theme was not installed");
			return;
		}
		auto pResult = SwitchThemesCommon::PatchBgLayouts(ToPatch, patch);
		if (pResult != BflytFile::PatchResult::OK)
		{
			Dialog("PatchBgLayouts failed for " + BaseSzs + "\nThe theme was not installed");
			return;
		}
		pResult = SwitchThemesCommon::PatchBntx(ToPatch, SData.files["image.dds"], patch);
		if (pResult != BflytFile::PatchResult::OK)
		{
			Dialog("PatchBntx failed for " + BaseSzs + "\nThe theme was not installed");
			return;
		}
		if (SData.files.count("layout.json"))
		{
			auto JsonBinary = SData.files["layout.json"];
			string JSON(reinterpret_cast<char*>(JsonBinary.data()), JsonBinary.size());
			auto patch = Patches::LoadLayout(JSON);
			if (!patch.IsCompatible(ToPatch))
			{
				Dialog("The provided layout is not compatible with " + BaseSzs + "\nThe theme was not installed");
				return;
			}
			auto res = SwitchThemesCommon::PatchLayouts(ToPatch, patch.Files);
			if (res != BflytFile::PatchResult::OK)
			{
				Dialog("PatchLayouts failed for " + BaseSzs + "\nThe theme was not installed");
				return;				
			}
		}
		auto packed = SARC::Pack(ToPatch);
		BaseFile = Yaz0::Compress(packed.data, 3, packed.align);
		CreateThemeStructure(patch.TitleId);
		WriteFile(CfwFolder + "/titles/" + patch.TitleId + "/romfs/lyt/" + patch.szsName, BaseFile);
	}
	Dialog("Done, restart the console to see the changes");
}

void ThemeEntry::ParseTheme()
{
	file = OpenFile(FileName);
	DecompressedFile = Yaz0::Decompress(file);	
	SData = SARC::Unpack(DecompressedFile);
	if (LegacyTheme())
		ParseLegacyTheme();
	else 
		ParseNxTheme();
}

void ThemeEntry::ParseNxTheme()
{
	auto themeInfo = ParseNXThemeFile(SData);
	if (themeInfo.Version == -1)
	{
		lblLine1.SetString("Invalid theme");
		CanInstall = false;
	}
	if (themeInfo.Version > 1)
	{
		lblLine2.SetString("New version, update the installer !");
		CanInstall = false;
	}		
	if (!ThemeTargetToName.count(themeInfo.Target))
	{
		lblLine2.SetString("Error: target not found");
		CanInstall = false;		
	}
	lblLine2.SetString(ThemeTargetToName[themeInfo.Target]);
	
	lblFname.SetString(themeInfo.ThemeName);
	string l1 = "";
	if (themeInfo.Author != "")
		l1 += "by " + themeInfo.Author;
	if (themeInfo.LayoutInfo != "")
	{
		if (l1 != "")
			l1 += " - ";
		l1 += "Layout: " + themeInfo.LayoutInfo;
	}
	
	if (l1 == "") //if meta is missing
		lblLine1.SetString(FileName); 
	lblLine1.SetString(l1);
}

void ThemeEntry::ParseLegacyTheme()
{
	lblFname.SetString(FileName.substr(FileName.find_last_of("/\\") + 1));
	lblLine1.SetString(FileName);
	auto patch = SwitchThemesCommon::DetectSarc(SData);
	if (patch.FirmName == "")
	{
		lblLine2.SetString("Invalid theme");
		CanInstall = false;
	}
	else lblLine2.SetString(patch.TemplateName + " for " + patch.FirmName);
}
		
void ThemeEntry::Render(int X, int Y, bool selected)
{
	SDL_Rect aSize = Size;
	if (selected)
	{
		aSize.x = X - 2; 
		aSize.y = Y - 2;
		aSize.w += 4; 
		aSize.h += 4; 
		SDL_SetRenderDrawColor(sdl_render,11,255,209,0xff);
		SDL_RenderFillRect(sdl_render,&aSize);
		aSize = Size;
	}
	aSize.x = X; aSize.y = Y;
	SDL_SetRenderDrawColor(sdl_render,67,67,67,0xff); 
	SDL_RenderFillRect(sdl_render,&aSize);
	lblFname.Render(X + 12, Y + 5);
	lblLine1.Render(X + 12, Y + 55);
	lblLine2.Render(X + EntryW - lblLine2.GetSize().w - 5, Y + 5);
}

SDL_Rect ThemeEntry::GetRect()
{
	return Size;
}