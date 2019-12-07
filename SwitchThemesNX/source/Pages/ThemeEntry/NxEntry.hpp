#include "ThemeEntry.hpp"
#include "../../fs.hpp"
#include "../../SwitchTools/PatchMng.hpp"
#include "../../SwitchThemesCommon/NXTheme.hpp"
#include "../SettingsPage.hpp"
#include "../../SwitchThemesCommon/Bntx/DDSconv/DDSConv.hpp"
#include "../../SwitchTools/hactool.hpp"
using namespace SwitchThemesCommon;

using namespace std;

class NxEntry : public ThemeEntry
{
public:
	NxEntry(const string& fileName, vector<u8>&& RawData)
	{
		FileName = fileName;
		auto DecompressedFile = Yaz0::Decompress(RawData);
		ParseNxTheme(SARC::Unpack(DecompressedFile));
	}

	NxEntry(const string& fileName, SARC::SarcData&& _SData)
	{
		FileName = fileName;
		ParseNxTheme(move(_SData));
	}

	bool IsFolder() override { return false; }
	bool CanInstall() override { return _CanInstall; }
	bool HasPreview() override { return _HasPreview; }
protected:

	bool DoInstall(bool ShowDialogs = true, const string& homeDirOverride = "") override
	{
		auto themeInfo = ParseNXThemeFile(SData);

		if (!PatchMng::ExefsCompatAsk(ThemeTargetToFileName[themeInfo.Target]))
			return false;

		string BaseSzs = SD_PREFIX "/themes/systemData/" + ThemeTargetToFileName[themeInfo.Target];
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

		if (ShowDialogs)
			DisplayLoading("Installing...");

		//common.szs patching code. Called if we are patching applets on <= 5.0 or there's a common layout
		//On <= 5.0 apply the background image for the applets
		bool ShouldPatchBGInCommon = HOSVer.major <= 5 && (themeInfo.Target == "news" || themeInfo.Target == "apps" || themeInfo.Target == "set");
		if ((themeInfo.Target == "home" && SData.files.count("common.json") && Settings::UseCommon) || ShouldPatchBGInCommon)
		{
			string CommonSzs = SD_PREFIX "/themes/systemData/common.szs";
			if (!filesystem::exists(CommonSzs))
			{
				MissingFileErrorDialog("common.szs");
				return false;
			}

			SARC::SarcData sarc;
			if (!SarcOpen(CommonSzs, &sarc)) return false;
			SzsPatcher Patcher(sarc);

			if (ShouldPatchBGInCommon)
			{
				if (NxThemeGetBgImage().size() != 0)
					if (!PatchBG(Patcher, NxThemeGetBgImage(), CommonSzs))
						return false;
			}

			if (SData.files.count("common.json") && themeInfo.Target == "home" && Settings::UseCommon)
			{
				auto JsonBinary = SData.files["common.json"];
				string JSON(reinterpret_cast<char*>(JsonBinary.data()), JsonBinary.size());
				if (!PatchLayout(Patcher, JSON, "common.szs"))
					return false;
			}

			if (homeDirOverride != "")
				fs::WriteFile(homeDirOverride + "common.szs", SarcPack(Patcher.GetFinalSarc()));
			else
			{
				fs::CreateThemeStructure("0100000000001000");
				fs::WriteFile(fs::GetFsMitmFolder() + "/0100000000001000/romfs/lyt/common.szs", SarcPack(Patcher.GetFinalSarc()));
			}
		}

		//Actual file patching code 
		bool FileHasBeenPatched = false;
		SARC::SarcData sarc;
		if (!SarcOpen(BaseSzs, &sarc)) return false;
		SzsPatcher Patcher(sarc);
		string TitleId = "0100000000001000";
		string SzsName = ThemeTargetToFileName[themeInfo.Target];
		auto patch = Patcher.DetectSarc();
		if (patch.FirmName != "")
		{
			TitleId = patch.TitleId;
			SzsName = patch.szsName;
		}

		if (!ShouldPatchBGInCommon)
		{
			if (patch.FirmName == "")
			{
				DialogBlocking("Couldn't find any patch for " + BaseSzs + "\nThe theme was not installed");
				return false;
			}
			if (NxThemeGetBgImage().size() != 0)
				if (!PatchBG(Patcher, NxThemeGetBgImage(), BaseSzs))
					return false;
				else FileHasBeenPatched = true;
		}

		/*
			The layout patching step has been moved after the custom user icons (and furutre home menu components)
			to let layouts edit the built-in patches that are applied to the panes. To avoid breaking old layouts
			patches from pre 9 nxthemes will still be applied first
		*/
#define APPLY_LAYOUT_PATCH do { \
if (SData.files.count("layout.json"))\
	{\
		auto JsonBinary = SData.files["layout.json"];\
		string JSON(reinterpret_cast<char*>(JsonBinary.data()), JsonBinary.size());\
		if (!PatchLayout(Patcher, JSON, themeInfo.Target))	return false;\
		FileHasBeenPatched = true;\
	} \
} while (0)

		if (NXThemeVer <= 8)
			APPLY_LAYOUT_PATCH;

		//Applet icons patching
		if (Settings::UseIcons)
		{
			if (NXThemeVer >= 8) {
				//New applet texture patching method
				if (Settings::UseIcons && Patches::textureReplacement::NxNameToList.count(themeInfo.Target))
				{
					for (const TextureReplacement& p : Patches::textureReplacement::NxNameToList[themeInfo.Target])
					{
						auto pResult = false;
						if (SData.files.count(p.NxThemeName + ".dds"))
							pResult = Patcher.PatchAppletIcon(SData.files[p.NxThemeName + ".dds"], p.NxThemeName);
						else if (SData.files.count(p.NxThemeName + ".png"))
						{
							auto dds = DDSConv::ImageToDDS(SData.files[p.NxThemeName + ".png"], true, p.W, p.H);
							if (dds.size() != 0)
								pResult = Patcher.PatchAppletIcon(dds, p.NxThemeName);
							else
							{
								DialogBlocking("Couldn't load the icon image for " + p.NxThemeName);
								continue;
							}
						}
						else continue;

						if (!pResult)
							DialogBlocking(p.NxThemeName + " icon patch failed for " + SzsName + "\nThe theme will be installed anyway but may crash.");
						else
							FileHasBeenPatched = true;
					}
				}
			}
			else
			{
				//Old album.szs patching to avoid breaking old themes
				if (themeInfo.Target == "home" && SData.files.count("album.dds"))
				{
					FileHasBeenPatched = true;
					if (!Patcher.PatchBntxTexture(SData.files["album.dds"], "RdtIcoPvr_00^s", 0x02000000))
						DialogBlocking("Album icon patch failed for " + SzsName + "\nThe theme will be installed anyway but may crash.");
				}
			}
		}

		if (NXThemeVer >= 9)
			APPLY_LAYOUT_PATCH;

		if (FileHasBeenPatched)
		{
			if (TitleId == "0100000000001000" && homeDirOverride != "")
				fs::WriteFile(homeDirOverride + SzsName, SarcPack(Patcher.GetFinalSarc()));
			else {
				fs::CreateThemeStructure(TitleId);
				fs::WriteFile(fs::GetFsMitmFolder() + "/" + TitleId + "/romfs/lyt/" + SzsName, SarcPack(Patcher.GetFinalSarc()));
			}
		}

		return true;
	}

	LoadedImage GetPreview() override
	{
		if (!_HasPreview) return 0;
		auto& image = NxThemeGetBgImage();
		if (image.size() == 0) return 0;
		auto Preview = ImageCache::LoadDDS(image, FileName);
		if (!Preview)
		{
			_HasPreview = false;
			DialogBlocking("Failed to load the preview image");
		}
		return Preview;
	}

private:
	bool _CanInstall = true;
	SARC::SarcData SData;
	bool _HasPreview = true;
	int NXThemeVer = 0;

	const vector<u8>& NxThemeGetBgImage()
	{
		if (!_HasPreview || !CanInstall()) return ThemeEntry::_emtptyVec;
		if (SData.files.count("image.dds"))
			return SData.files["image.dds"];
		else if (SData.files.count("image.jpg"))
		{
			auto res = DDSConv::ImageToDDS(SData.files["image.jpg"], false, 1280, 720);
			if (res.size() != 0)
			{
				//HACK: don't save the nxtheme after this
				SData.files["image.dds"] = res;
				_HasPreview = true;
				return SData.files["image.dds"];
			}
			else
			{
				_HasPreview = false;
				_CanInstall = false;
				lblLine2 = DDSConv::GetError();
				return  ThemeEntry::_emtptyVec;
			}
		}
		return ThemeEntry::_emtptyVec;
	}

	void ParseNxTheme(SARC::SarcData&& _Sdata)
	{
		SData = move(_Sdata);
		file.clear(); //we don't need the full file for nxthemes
		auto themeInfo = ParseNXThemeFile(SData);
		if (themeInfo.Version == -1)
		{
			lblLine1 = ("Invalid theme");
			_CanInstall = false;
		}
		NXThemeVer = themeInfo.Version;
		if (themeInfo.Version > SwitchThemesCommon::NXThemeVer)
		{
			lblLine2 = ("New version, update the installer !");
			_CanInstall = false;
		}
		if (_CanInstall) {
			if (SData.files.count("image.dds") || SData.files.count("image.jpg"))
			{
				_HasPreview = true;
			}
		}
		if (!ThemeTargetToName.count(themeInfo.Target))
		{
			lblLine2 = ("Error: target not found");
			_CanInstall = false;
		}
		else if (_CanInstall)
		{
			string targetStr = ThemeTargetToName[themeInfo.Target];
			if (_HasPreview)
				targetStr += " - press X for preview";
			lblLine2 = (targetStr);
		}

		lblFname = (themeInfo.ThemeName);
		string l1 = "";
		if (themeInfo.Author != "")
			l1 += "by " + themeInfo.Author;
		if (themeInfo.LayoutInfo != "")
		{
			l1 += " - " + themeInfo.LayoutInfo;
		}

		if (l1 == "") //if meta is missing
			lblLine1 = (FileName);
		lblLine1 = (l1);
	}


	static bool PatchBG(SzsPatcher& Patcher, const vector<u8>& data, const string& SzsName)
	{
		if (!Patcher.PatchMainBG(data))
		{
			DialogBlocking("PatchBntx failed for " + SzsName + "\nThe theme was not installed");
			return false;
		}
		return true;
	}

	static bool PatchLayout(SzsPatcher& Patcher, const string& JSON, const string& PartName)
	{
		auto patch = Patches::LoadLayout(JSON);
		if (!patch.IsCompatible(Patcher.GetSarc()))
		{
			DialogBlocking("The provided layout is not compatible with " + PartName + "\nThe theme was not installed");
			return false;
		}
		Patcher.SetPatchAnimations(Settings::UseAnimations);
		if (!Patcher.PatchLayouts(patch, PartName, HOSVer.major >= 8 && PartName == "home"))
		{
			DialogBlocking("PatchLayouts failed for " + PartName + "\nThe theme was not installed");
			return false;
		}
		if (Settings::UseAnimations)
		{
			if (!Patcher.PatchAnimations(patch.Anims))
			{
				DialogBlocking("PatchAnimations failed for " + PartName + "\nThe theme was not installed");
				return false;
			}
		}
		return true;
	}

	static void MissingFileErrorDialog(const string& name)
	{
		DialogBlocking("Can't install this theme because the original " + name + " is missing from systemData.\n"
			"To install theme packs (.nxtheme files) you need to dump the home menu romfs following the guide in the \"Extract home menu\" tab");
	}

	static inline bool SarcOpen(const string& path, SARC::SarcData* out)
	{
		auto f = fs::OpenFile(path);
		if (f.size() == 0) return false;
		f = Yaz0::Decompress(f);
		*out = SARC::Unpack(f);
		return true;
	}

	static inline vector<u8> SarcPack(SARC::SarcData& data)
	{
		auto packed = SARC::Pack(data);
		return Yaz0::Compress(packed.data, 3, packed.align);
	}
};