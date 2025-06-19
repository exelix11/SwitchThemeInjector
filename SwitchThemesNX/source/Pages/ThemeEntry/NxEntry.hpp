#include "ThemeEntry.hpp"
#include "../../fs.hpp"
#include "../../SwitchTools/PatchMng.hpp"
#include "../../SwitchThemesCommon/NXTheme.hpp"
#include "../SettingsPage.hpp"
#include "../../SwitchThemesCommon/Bntx/DDSConv/DDSConv.hpp"
#include "../../SwitchTools/hactool.hpp"

class NxEntry : public ThemeEntry
{
public:
	NxEntry(const std::string& fileName, std::vector<u8>&& RawData)
	{
		FileName = fileName;
		auto DecompressedFile = Yaz0::Decompress(RawData);
		ParseNxTheme(SARC::Unpack(DecompressedFile));
	}

	NxEntry(const std::string& fileName, SARC::SarcData&& _SData)
	{
		FileName = fileName;
		ParseNxTheme(std::move(_SData));
	}

	bool IsFolder() override { return false; }
	bool CanInstall() override { return _CanInstall; }
	bool HasPreview() override { return _HasPreview; }
protected:

	const std::string_view StringFromVec(const std::vector<u8>& vec)
	{
		return std::string_view(reinterpret_cast<const char*>(vec.data()), vec.size());
	}

	bool DoInstall(bool ShowDialogs = true) override
	{
		auto themeInfo = ParseNXThemeFile(SData);

		if (!PatchMng::ExefsCompatAsk(ThemeTargetToFileName[themeInfo.Target]))
			return false;

		std::string BaseSzs = fs::path::SystemDataFolder + ThemeTargetToFileName[themeInfo.Target];
		if (!fs::Exists(BaseSzs))
		{
			try {
				if (themeInfo.Target == "user")
					hactool::ExtractUserPage();
				else if (themeInfo.Target == "psl")
					hactool::ExtractPlayerSelectMenu();
				
				goto CONTINUE_INSTALL;
			}
			catch (std::runtime_error& err)
			{
				DialogBlocking("Error while extracting the requested title: " + std::string(err.what()));
				return false;
			}

			MissingFileErrorDialog(ThemeTargetToFileName[themeInfo.Target]);
			return false;
		}
	CONTINUE_INSTALL:

		if (ShowDialogs)
			ThemeEntry::DisplayInstallDialog(FileName);

		const std::string CommonDestPath = fs::path::RomfsFolder("0100000000001000") + "lyt/common.szs";
		// If we're installing a resident menu theme delete the common.szs file that could be a leftover from a previous theme
		if (themeInfo.Target == "home" && fs::Exists(CommonDestPath))
			fs::Delete(CommonDestPath);

		// common.szs is patched in the following cases: 
		//	On <= 5.0 apply the background image for the applets
		bool ShouldPatchBGInCommon = HOSVer.major <= 5 && (themeInfo.Target == "news" || themeInfo.Target == "apps" || themeInfo.Target == "set");
		//	If we have a common layout and it's enabled in the settings 
		bool HasCommonLayout = themeInfo.Target == "home" && SData.files.count("common.json") && Settings::UseCommon;

		if (HasCommonLayout || ShouldPatchBGInCommon)
		{
			std::string CommonSzs = SYSTEMDATA_PATH "common.szs";
			if (!fs::Exists(CommonSzs))
			{
				MissingFileErrorDialog("common.szs");
				return false;
			}

			SARC::SarcData sarc;
			if (!SarcOpen(CommonSzs, &sarc)) return false;
			SwitchThemesCommon::SzsPatcher Patcher(sarc);

			if (ShouldPatchBGInCommon)
			{
				if (NxThemeGetBgImage().size() != 0)
					if (!PatchBG(Patcher, NxThemeGetBgImage(), CommonSzs))
						return false;
			}

			if (HasCommonLayout)
			{
				auto JsonBinary = SData.files["common.json"];
				if (!PatchLayout(Patcher, StringFromVec(JsonBinary), "common.szs"))
					return false;
			}

			fs::theme::CreateStructure("0100000000001000");
			fs::WriteFile(CommonDestPath, SarcPack(Patcher.GetFinalSarc()));
		}

		//Actual file patching code 
		bool FileHasBeenPatched = false;
		SARC::SarcData sarc;
		
		if (!SarcOpen(BaseSzs, &sarc)) 
			return false;
		
		SwitchThemesCommon::SzsPatcher Patcher(sarc);
		
		std::string ContentID = "0100000000001000";
		std::string SzsName = ThemeTargetToFileName[themeInfo.Target];
		auto patch = Patcher.DetectedSarc();

		// TODO: Why is there a fallback here ? if detect sarc fails we will not be able to apply the wallaper anyway
		if (patch)
		{
			ContentID = patch->TitleId;
			SzsName = patch->szsName;
		}

		if (!ShouldPatchBGInCommon)
		{
			if (!patch)
			{
				DialogBlocking("Couldn't find any patch for " + BaseSzs + "\nThe theme was not installed");
				return false;
			}

			if (NxThemeGetBgImage().size() != 0)
			{
				if (!PatchBG(Patcher, NxThemeGetBgImage(), BaseSzs))
					return false;

				FileHasBeenPatched = true;
			}
		}

		/*
			The layout patching step has been moved after the custom user icons (and future home menu components)
			to let layouts edit the built-in patches that are applied to the panes. To avoid breaking old layouts
			patches from pre 9 nxthemes will still be applied first
		*/
		const auto applyLayoutPatch = [&](){
			if (SData.files.count("layout.json"))
			{
				auto JsonBinary = SData.files["layout.json"]; 
				
				if (!PatchLayout(Patcher, StringFromVec(JsonBinary), themeInfo.Target))	
					return false; 
				
				FileHasBeenPatched = true; 
			} 
			else if (
				Settings::HomeMenuCompat == SwitchThemesCommon::LayoutCompatibilityOption::Firmware10 || 
				Settings::HomeMenuCompat == SwitchThemesCommon::LayoutCompatibilityOption::Firmware11
			)
			{
 				// Special case: this theme has no layout but the user requested to force compatibility fixes
				if (!PatchLayout(Patcher, "", themeInfo.Target))
					return false;

				FileHasBeenPatched = true;
			}

			return true;
		};

		if (NXThemeVer <= 8)
		{
			if (!applyLayoutPatch())
				return false;
		}

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
					if (!Patcher.PatchBntxTexture(SData.files["album.dds"], {"RdtIcoPvr_00^s"}, 0x02000000))
						DialogBlocking("Album icon patch failed for " + SzsName + "\nThe theme will be installed anyway but may crash.");
				}
			}
		}

		if (NXThemeVer >= 9)
		{
			if (!applyLayoutPatch())
				return false;
		}

		if (FileHasBeenPatched)
		{			
			fs::theme::CreateStructure(ContentID);
			fs::WriteFile(fs::path::RomfsFolder(ContentID) +  "lyt/" + SzsName, SarcPack(Patcher.GetFinalSarc()));
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
	bool _HasPreview = false;
	int NXThemeVer = 0;

	const std::vector<u8>& NxThemeGetBgImage()
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
				CannotInstallReason = "Couldn't convert the included image " + lblLine2;
				return  ThemeEntry::_emtptyVec;
			}
		}
		return ThemeEntry::_emtptyVec;
	}

	void ParseNxTheme(SARC::SarcData&& _Sdata)
	{
		SData = std::move(_Sdata);
		file.clear(); //we don't need the full file for nxthemes
		auto themeInfo = ParseNXThemeFile(SData);
		if (themeInfo.Version == -1)
		{
			lblLine1 = "Invalid theme";
			CannotInstallReason = "Invalid theme";
			_CanInstall = false;
		}
		NXThemeVer = themeInfo.Version;
		if (themeInfo.Version > SwitchThemesCommon::NXThemeVer)
		{
			lblLine2 = "New version, update the installer !";
			CannotInstallReason = "This theme requres a newer version of the theme installer. Download latest version from GitHub.";
			_CanInstall = false;
		}
		if (_CanInstall) {
			if (SData.files.count("image.dds") || SData.files.count("image.jpg"))
				_HasPreview = true;
		}
		if (!ThemeTargetToName.count(themeInfo.Target))
		{
			lblLine2 = "Error: invalid target";
			CannotInstallReason = "The target home menu part is not valid";
			_CanInstall = false;
		}
		else if (_CanInstall)
		{
			std::string targetStr = ThemeTargetToName[themeInfo.Target];
			if (_HasPreview)
				targetStr += " - press X for preview";
			lblLine2 = (targetStr);
		}

		lblFname = (themeInfo.ThemeName);
		std::string l1 = "";
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


	static bool PatchBG(SwitchThemesCommon::SzsPatcher& Patcher, const std::vector<u8>& data, const std::string& SzsName)
	{
		if (!Patcher.PatchMainBG(data))
		{
			DialogBlocking("PatchBntx failed for " + SzsName + "\nThe theme was not installed");
			return false;
		}
		return true;
	}

	bool PatchLayout(SwitchThemesCommon::SzsPatcher& Patcher, const std::string_view JSON, const std::string& PartName)
	{
		Patcher.CompatFixes = Settings::HomeMenuCompat;
		bool success = false;

		// Special case, apply compat fixes to old themes that have no layout if manually requested by the user
		if (JSON.empty())
		{
			success = Patcher.PatchLayouts();
		}
		else
		{
			auto patch = Patches::LoadLayout(JSON);
			success = Patcher.PatchLayouts(patch, PartName);
		}

		if (!success)
		{
			DialogBlocking("PatchLayouts failed for " + PartName + "\nThe theme was not installed");
			return false;
		}

		if (Patcher.TotalNonCompatibleFixes > 0)
		{
			AppendInstallMessage("The theme \"" + lblFname + "\" for " + PartName + " contained a custom layout that was not compatible with your current firmware, the problematic parts were automatically removed and might change the look of the theme.");
		}

		return true;
	}

	static void MissingFileErrorDialog(const std::string& name)
	{
		DialogBlocking("Can't install this theme because the original " + name + " is missing from systemData.\n"
			"To install theme packs (.nxtheme files) you need to dump the home menu romfs following the guide in the \"Extract home menu\" tab");
	}

	static inline bool SarcOpen(const std::string& path, SARC::SarcData* out)
	{
		auto f = fs::OpenFile(path);
		if (f.size() == 0) return false;
		f = Yaz0::Decompress(f);
		*out = SARC::Unpack(f);
		return true;
	}

	static inline std::vector<u8> SarcPack(SARC::SarcData& data)
	{
		auto packed = SARC::Pack(data);
		return Yaz0::Compress(packed.data, 3, packed.align);
	}
};