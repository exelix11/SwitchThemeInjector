#include "ThemeEntry.hpp"
#include "../SwitchThemesCommon/NXTheme.hpp"
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../ViewFunctions.hpp"
#include "../fs.hpp"
#include <filesystem>

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
	if (filesystem::is_directory(fileName))
	{
		lblFname.SetString(GetFileName(FileName));
		lblLine1.SetString(FileName);
		lblLine2.SetString("Folder");
		CanInstall = false;
		IsFolder = true;
	}
	else ParseTheme();
}

bool ThemeEntry::LegacyTheme()
{
	return StrEndsWith(FileName,".szs");
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
	lblFname.SetString(GetFileName(FileName));
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

bool PatchBG(SARC::SarcData &ToPatch, const PatchTemplate &patch, const vector<u8> &data, const string &SzsName)
{
	auto pResult = SwitchThemesCommon::PatchBgLayouts(ToPatch, patch);
	if (pResult != BflytFile::PatchResult::OK)
	{
		Dialog("PatchBgLayouts failed for " + SzsName + "\nThe theme was not installed");
		return false;
	}
	
	pResult = SwitchThemesCommon::PatchBntx(ToPatch, data, patch);
	if (pResult != BflytFile::PatchResult::OK)
	{
		Dialog("PatchBntx failed for " + SzsName + "\nThe theme was not installed");
		return false;
	}
	return true;
}

bool PatchLayout(SARC::SarcData &ToPatch, const string &JSON, const string &SzsName)
{
	auto patch = Patches::LoadLayout(JSON);
	if (!patch.IsCompatible(ToPatch))
	{
		Dialog("The provided layout is not compatible with " + SzsName + "\nThe theme was not installed");
		return false;
	}
	auto res = SwitchThemesCommon::PatchLayouts(ToPatch, patch.Files);
	if (res != BflytFile::PatchResult::OK)
	{
		Dialog("PatchLayouts failed for " + SzsName + "\nThe theme was not installed");
		return false;				
	}
	return true;
}

void MissingFileErrorDialog(const string &name)
{
	Dialog(	"Can't install this theme because the original " + name + " is missing from systemData.\n"
			"To install theme packs (.nxtheme files) you need to dump the home menu romfs following the guide in the \"NCA extraction\" tab");
}

inline SARC::SarcData SarcOpen(const string &path)
{
	auto f = OpenFile(path);
	f = Yaz0::Decompress(f);
	return SARC::Unpack(f);
}

inline vector<u8> SarcPack(SARC::SarcData &data)
{
	auto packed = SARC::Pack(data);
	return Yaz0::Compress(packed.data, 3, packed.align);
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
		string szsPath = CfwFolder + "/titles/" + patch.TitleId + "/romfs/lyt/" + patch.szsName;
		WriteFile(szsPath, file);
	}
	else 
	{
		auto themeInfo = ParseNXThemeFile(SData);
		string BaseSzs = "/themes/systemData/" + ThemeTargetToFileName[themeInfo.Target];
		if (!filesystem::exists(BaseSzs))
		{
			MissingFileErrorDialog(ThemeTargetToFileName[themeInfo.Target]);
			return;
		}
		DisplayLoading("Installing...");
		
		auto ToPatch = SarcOpen(BaseSzs);
		auto patch = SwitchThemesCommon::DetectSarc(ToPatch);
		if (patch.FirmName == "")
		{
			Dialog("Couldn't find any patch for " + BaseSzs + "\nThe theme was not installed");
			return;
		}
		
		bool Patched5x = false; //If just the bg gets patched don't save the ResidentMenu file later
		if (themeInfo.Target == "home" && patch.FirmName == "<= 5.X" && themeInfo.UseCommon5X)
		{
			Patched5x = true;
			
			string CommonSzs = "/themes/systemData/common.szs";
			if (!filesystem::exists(CommonSzs))
			{
				MissingFileErrorDialog("common.szs");
				return;
			}
			
			auto CommonSarc = SarcOpen(CommonSzs);
			auto CommonPatch = SwitchThemesCommon::DetectSarc(CommonSarc);
			
			if (!PatchBG(CommonSarc, CommonPatch, SData.files["image.dds"],CommonSzs))
				return;
			
			CreateThemeStructure(CommonPatch.TitleId);
			WriteFile(CfwFolder + "/titles/" + CommonPatch.TitleId + "/romfs/lyt/common.szs", SarcPack(CommonSarc));
		}
		else 
		{		
			if (!PatchBG(ToPatch, patch, SData.files["image.dds"],BaseSzs))
				return;
		}
				
		if (SData.files.count("layout.json"))
		{
			Patched5x = false;
			auto JsonBinary = SData.files["layout.json"];
			string JSON(reinterpret_cast<char*>(JsonBinary.data()), JsonBinary.size());
			if (!PatchLayout(ToPatch,JSON,BaseSzs))
				return;
		}
		
		if (!Patched5x)
		{
			CreateThemeStructure(patch.TitleId);
			WriteFile(CfwFolder + "/titles/" + patch.TitleId + "/romfs/lyt/" + patch.szsName, SarcPack(ToPatch));
		}
	}
	Dialog("Done, restart the console to see the changes");
}

SDL_Rect ThemeEntry::GetRect()
{
	return Size;
}