#include "ThemeEntry.hpp"
#include "../SwitchThemesCommon/NXTheme.hpp"
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../ViewFunctions.hpp"
#include "../fs.hpp"
#include "../input.hpp"
#include "../SwitchTools/hactool.hpp"
#include <filesystem>

using namespace std;

const int EntryW = 860;
const int EntryH = 96;

ThemeEntry::~ThemeEntry()
{
	if (Preview)
	{
		delete Preview;
	}
}

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
	else
	{
		file = OpenFile(FileName);
		ParseTheme();
	}
}

ThemeEntry::ThemeEntry(const vector<u8> &RawData):
	lblFname("", WHITE, -1,font30),
	lblLine1("", WHITE, -1,font25),
	lblLine2("", WHITE, -1,font25),
	Size {0,0, EntryW, EntryH }
{
	FileName = "";
	file = RawData;
	ParseTheme();
}

bool ThemeEntry::LegacyTheme()
{
	return StrEndsWith(FileName,".szs") || !SData.files.count("info.json");
}

bool ThemeEntry::IsFont()
{
	return StrEndsWith(FileName,".ttf");
}

void ThemeEntry::ParseTheme()
{	
	if (IsFont())
	{
		ParseFont();
		return;
	}
	DecompressedFile = Yaz0::Decompress(file);	
	SData = SARC::Unpack(DecompressedFile);
	if (LegacyTheme())
		ParseLegacyTheme();
	else
		ParseNxTheme();
}

void ThemeEntry::ParseFont()
{
	lblLine2.SetString("Custom font");
	auto fontName = SwitchThemesCommon::TTF::GetFontName(file);
	CanInstall = fontName != "";
	lblFname.SetString(CanInstall ? fontName : "Invalid font :(");
	lblLine1.SetString(GetFileName(FileName));
}

void ThemeEntry::ParseNxTheme()
{
	auto themeInfo = ParseNXThemeFile(SData);
	if (themeInfo.Version == -1)
	{
		lblLine1.SetString("Invalid theme");
		CanInstall = false;
	}
	if (themeInfo.Version > 7)
	{
		lblLine2.SetString("New version, update the installer !");
		CanInstall = false;
	}		
	if (SData.files.count("preview.png"))
	{
		NXThemeHasPreview = true;
	}	
	if (!ThemeTargetToName.count(themeInfo.Target))
	{
		lblLine2.SetString("Error: target not found");
		CanInstall = false;		
	}
	else
	{
		string targetStr = ThemeTargetToName[themeInfo.Target];
		if (NXThemeHasPreview)
			targetStr += " - press L for preview";
		lblLine2.SetString(targetStr);
	}
	
	lblFname.SetString(themeInfo.ThemeName);
	string l1 = "";
	if (themeInfo.Author != "")
		l1 += "by " + themeInfo.Author;
	if (themeInfo.LayoutInfo != "")
	{
		l1 += " - " + themeInfo.LayoutInfo;
	}
	
	if (l1 == "") //if meta is missing
		lblLine1.SetString(FileName); 
	lblLine1.SetString(l1);
}

void ThemeEntry::ParseLegacyTheme()
{
	if (FileName == "")
	{
		lblFname.SetString("Unknown.szs");
		lblLine1.SetString("Remote install");
	}
	else
	{
		lblFname.SetString(GetFileName(FileName));
		lblLine1.SetString(FileName);		
	}
	auto patch = SwitchThemesCommon::DetectSarc(SData);
	if (patch.FirmName == "")
	{
		lblLine2.SetString("Invalid theme");
		CanInstall = false;
	}
	else lblLine2.SetString(patch.TemplateName + " for " + patch.FirmName);
}

void ThemeEntry::NXLoadPreview()
{
	Preview = new Image(SData.files["preview.png"]);
	Preview->ImageSetSize(1280,720);
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
		
		if (NXThemeHasPreview && (kHeld & KEY_L))
		{
			if (!Preview)
				NXLoadPreview();
			if (Preview)
			{
				Preview->Render(0,0);
				return;
			}
		}
	}
	aSize.x = X; aSize.y = Y;
	if (Highlighted)
		SDL_SetRenderDrawColor(sdl_render,0x36,0x6e,0x64,0xff); 
	else 
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

bool PatchLayout(SARC::SarcData &ToPatch, const string &JSON, const string &PartName)
{
	auto patch = Patches::LoadLayout(JSON);
	if (!patch.IsCompatible(ToPatch))
	{
		Dialog("The provided layout is not compatible with " + PartName + "\nThe theme was not installed");
		return false;
	}
	auto res = SwitchThemesCommon::PatchLayouts(ToPatch, patch, PartName, NXTheme_FirmMajor >= 8 && PartName == "home" , UseAnimations);
	if (res != BflytFile::PatchResult::OK)
	{
		Dialog("PatchLayouts failed for " + PartName + "\nThe theme was not installed");
		return false;				
	}
	if (UseAnimations)
	{
		res = SwitchThemesCommon::PatchAnimations(ToPatch, patch.Anims);
		if (res != BflytFile::PatchResult::OK)
		{
			Dialog("PatchAnimations failed for " + PartName + "\nThe theme was not installed");
			return false;				
		}
	}
	return true;
}

void MissingFileErrorDialog(const string &name)
{
	Dialog(	"Can't install this theme because the original " + name + " is missing from systemData.\n"
			"To install theme packs (.nxtheme files) you need to dump the home menu romfs following the guide in the \"Extract home menu\" tab");
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

bool ThemeEntry::InstallTheme(bool ShowLoading, const string &homeDirOverride) 
{
	if (!CanInstall)
	{
		Dialog("Can't install this theme, check that it hasn't been corrupted and that you are using an updated version of this installer");
		return false;
	}
	try {
		if (IsFont())
		{
			if (homeDirOverride != "")
			{
				DialogBlocking("Can't install a font to theme shuffle.");
				return false;
			}
			
			if (ShowLoading)
				DisplayLoading("Installing font...");
			
			CreateFsMitmStructure("0100000000000811");
			CreateRomfsDir("0100000000000811");
			WriteFile(CfwFolder + "/titles/0100000000000811/romfs/nintendo_udsg-r_std_003.bfttf", SwitchThemesCommon::TTF::ConvertToBFTTF(file));
			CreateFsMitmStructure("0100000000000039");
			CreateRomfsDir("0100000000000039");
			WriteFile(CfwFolder + "/titles/0100000000000039/romfs/dummy.bin", {0x70,0x61,0x70,0x65,0x20,0x53,0x61,0x74,0x61,0x6E,0x20,0x41,0x6C,0x65,0x70,0x70,0x65,0x21});
		}
		else if (LegacyTheme())
		{
			if (ShowLoading)
				DisplayLoading("Installing...");
			PatchTemplate patch = SwitchThemesCommon::DetectSarc(SData);
			CreateThemeStructure(patch.TitleId);
			string szsPath;
			if (patch.TitleId == "0100000000001000" && homeDirOverride != "")
				szsPath	= homeDirOverride + patch.szsName;
			else 
				szsPath	= CfwFolder + "/titles/" + patch.TitleId + "/romfs/lyt/" + patch.szsName;
			WriteFile(szsPath, file);
		}
		else 
		{
			auto themeInfo = ParseNXThemeFile(SData);
			string BaseSzs = "/themes/systemData/" + ThemeTargetToFileName[themeInfo.Target];
			if (!filesystem::exists(BaseSzs))
			{
				if (themeInfo.Target == "user" && ExtractUserPage())
					goto CONTINUE_INSTALL;
				if (themeInfo.Target == "psl" && ExtractPlayerSelectMenu())
					goto CONTINUE_INSTALL;
				
				MissingFileErrorDialog(ThemeTargetToFileName[themeInfo.Target]);
				return false;
			}
		CONTINUE_INSTALL:
			
			if (ShowLoading)
				DisplayLoading("Installing...");
						
			bool DoPatchCommonBG = NXTheme_FirmMajor <= 5 && (themeInfo.Target == "news" || themeInfo.Target == "apps" || themeInfo.Target == "set"); //On 5.x these files must patch the bg in common
			bool SkipSaveActualFile = false; //If the bg gets patched don't save the ResidentMenu file later
			if ((themeInfo.Target == "home" && SData.files.count("common.json")) || DoPatchCommonBG)
			{
				//common.szs patching code
				
				string CommonSzs = "/themes/systemData/common.szs";
				if (!filesystem::exists(CommonSzs))
				{
					MissingFileErrorDialog("common.szs");
					return false;
				}
				
				auto CommonSarc = SarcOpen(CommonSzs);
				
				if (DoPatchCommonBG)
				{
					SkipSaveActualFile = true; //Do not save resident if the bg has been applied to common
					if (SData.files.count("image.dds"))
					{
						auto CommonPatch = SwitchThemesCommon::DetectSarc(CommonSarc);
						if (!PatchBG(CommonSarc, CommonPatch, SData.files["image.dds"],CommonSzs))
							return false;
					}
				}
				
				if (SData.files.count("common.json") && themeInfo.Target == "home")
				{
					auto JsonBinary = SData.files["common.json"];
					string JSON(reinterpret_cast<char*>(JsonBinary.data()), JsonBinary.size());
					if (!PatchLayout(CommonSarc,JSON,"common.szs"))
						return false;
				}
				
				if (homeDirOverride != "")
					WriteFile(homeDirOverride + "common.szs", SarcPack(CommonSarc));
				else 
				{
					CreateThemeStructure("0100000000001000");
					WriteFile(CfwFolder + "/titles/0100000000001000/romfs/lyt/common.szs", SarcPack(CommonSarc));
				}
			}
			
			auto ToPatch = SarcOpen(BaseSzs);
			string TitleId = "0100000000001000";
			string SzsName = ThemeTargetToFileName[themeInfo.Target];
			auto patch = SwitchThemesCommon::DetectSarc(ToPatch);
			if (patch.FirmName != "")
			{
				TitleId = patch.TitleId;
				SzsName = patch.szsName;
			}

			if (!SkipSaveActualFile)
			{		
				if (patch.FirmName == "")
				{
					Dialog("Couldn't find any patch for " + BaseSzs + "\nThe theme was not installed");
					return false;
				}				
				if (SData.files.count("image.dds"))
					if (!PatchBG(ToPatch, patch, SData.files["image.dds"],BaseSzs))
						return false;
			}
					
			if (SData.files.count("layout.json"))
			{
				SkipSaveActualFile = false;
				auto JsonBinary = SData.files["layout.json"];
				string JSON(reinterpret_cast<char*>(JsonBinary.data()), JsonBinary.size());
				if (!PatchLayout(ToPatch, JSON, themeInfo.Target))
					return false;
			}
			
			if (themeInfo.Target == "home" && SData.files.count("album.dds"))
			{
				SkipSaveActualFile = false;
				auto pResult = SwitchThemesCommon::PatchBntxTexture(ToPatch, SData.files["album.dds"], "RdtIcoPvr_00^s", 0x02000000 );
				if (pResult != BflytFile::PatchResult::OK)
				{
					Dialog("Album icon patch failed for " + SzsName + "\nThe theme will be installed anyway but may crash.");
				}
			}
			
			if (!SkipSaveActualFile)
			{
				if (TitleId == "0100000000001000" && homeDirOverride != "")
					WriteFile(homeDirOverride + SzsName, SarcPack(ToPatch));
				else {
					CreateThemeStructure(TitleId);
					WriteFile(CfwFolder + "/titles/" + TitleId + "/romfs/lyt/" + SzsName, SarcPack(ToPatch));
				}
			}
		}
		if (ShowLoading)
			Dialog("Done, restart the console to see the changes");
	}
	catch (const char * err)
	{
		Dialog("Error while installing this theme: " + string(err));
		return false;
	}
	catch (const string &err)
	{
		Dialog("Error while installing this theme: " + err);	
		return false;	
	}
	catch (...)
	{
		Dialog("Exception while installing this theme.");	
		return false;
	}
	return true;
}

SDL_Rect ThemeEntry::GetRect()
{
	return Size;
}