#include "ThemeEntry.hpp"
#include "../SwitchThemesCommon/NXTheme.hpp"
#include "../SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../ViewFunctions.hpp"
#include "../fs.hpp"
#include "../SwitchTools/hactool.hpp"
#include <filesystem>
#include "../Platform/Platform.hpp"
#include "../SwitchThemesCommon/Bntx/DDSconv/DDSConv.hpp"

#include "SettingsPage.hpp"
#include "../SwitchTools/PatchMng.hpp"

using namespace std;
using namespace SwitchThemesCommon;

ThemeEntry::~ThemeEntry()
{
	if (NXThemeHasPreview)
		ImageCache::FreeImage(FileName);
}

ThemeEntry::ThemeEntry(const string &fileName)
{
	FileName = fileName;
	if (filesystem::is_directory(fileName))
	{
		lblFname = (fs::GetFileName(FileName));
		lblLine1 = (FileName);
		lblLine2 = ("Folder");
		CanInstall = false;
		IsFolder = true;
	}
	else
	{
		file = fs::OpenFile(FileName);
		ParseTheme();
	}
}

ThemeEntry::ThemeEntry(const vector<u8> &RawData)
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
	if (file.size() == 0)
	{
		CanInstall = false;
		lblLine1 = FileName;
		lblFname = "Couldn't open this file";
		return;
	}

	if (IsFont())
	{
		ParseFont();
		return;
	}
	auto DecompressedFile = Yaz0::Decompress(file);
	SData = SARC::Unpack(DecompressedFile);
	if (LegacyTheme())
		ParseLegacyTheme();
	else
		ParseNxTheme();
}

void ThemeEntry::ParseFont()
{
	lblLine2 = ("Custom font");
	auto fontName = SwitchThemesCommon::TTF::GetFontName(file);
	CanInstall = fontName != "";
	lblFname = (CanInstall ? fontName : "Invalid font :(");
	lblLine1 = (fs::GetFileName(FileName));
}

void ThemeEntry::ParseNxTheme()
{
	file.clear(); //we don't need the full file for nxthemes
	auto themeInfo = ParseNXThemeFile(SData);
	if (themeInfo.Version == -1)
	{
		lblLine1 = ("Invalid theme");
		CanInstall = false;
	}
	NXThemeVer = themeInfo.Version;
	if (themeInfo.Version > SwitchThemesCommon::NXThemeVer)
	{
		lblLine2 = ("New version, update the installer !");
		CanInstall = false;
	}		
	if (CanInstall) {
		if (SData.files.count("image.dds") || SData.files.count("image.jpg"))
		{
			NXThemeHasPreview = true;
		}
	}
	if (!ThemeTargetToName.count(themeInfo.Target))
	{
		lblLine2 = ("Error: target not found");
		CanInstall = false;		
	}
	else if (CanInstall)
	{
		string targetStr = ThemeTargetToName[themeInfo.Target];
		if (NXThemeHasPreview)
			targetStr += " - press L for preview";
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

vector<u8> ThemeEntry::NxThemeGetBgImage()
{
	if (!NXThemeHasPreview || !CanInstall) return {};
	if (SData.files.count("image.dds"))
		return SData.files["image.dds"];
	else if (SData.files.count("image.jpg"))
	{
		auto res = DDSConv::ImageToDDS(SData.files["image.jpg"], false, 1280, 720);
		if (res.size() != 0)
		{
			//HACK: don't save the nxtheme after this
			SData.files["image.dds"] = res;
			NXThemeHasPreview = true;
			return SData.files["image.dds"];
		}
		else
		{
			NXThemeHasPreview = false;
			CanInstall = false;
			lblLine2 = DDSConv::GetError();
			return {};
		}
	}
	return {};
}

void ThemeEntry::ParseLegacyTheme()
{
	if (FileName == "")
	{
		lblFname = ("Unknown.szs");
		lblLine1 = ("Remote install");
	}
	else
	{
		lblFname = (fs::GetFileName(FileName));
		lblLine1 = (FileName);		
	}
	auto patch = SwitchThemesCommon::SzsPatcher::DetectSarc(SData);
	if (patch.FirmName == "")
	{
		lblLine2 = ("Invalid theme");
		CanInstall = false;
	}
	else lblLine2 = (patch.TemplateName + " for " + patch.FirmName);
}

LoadedImage ThemeEntry::NXGetPreview()
{
	if (!NXThemeHasPreview) return 0;
	auto image = NxThemeGetBgImage();
	if (image.size() == 0) return 0;
	auto Preview = ImageCache::LoadDDS(image, FileName);
	if (!Preview)
	{
		NXThemeHasPreview = false;
		Dialog("Failed to load the preview image");
	}
	return Preview;
}

using namespace ImGui;
bool ThemeEntry::IsHighlighted() 
{
	return GImGui->NavId == GetCurrentWindow()->GetID(FileName.c_str());
}

ThemeEntry::UserAction ThemeEntry::Render(bool OverrideColor)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return UserAction::None;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(FileName.c_str());

	ImGui::PushFont(font30);
	const ImVec2 name_size = CalcTextSize(lblFname.c_str(), NULL, false);
	ImGui::PopFont();
	ImGui::PushFont(font25);
	const ImVec2 line1_size = CalcTextSize(lblLine1.c_str(), NULL, false, EntryW - 5);
	const ImVec2 line2_size = CalcTextSize(lblLine2.c_str(), NULL, false);
	ImGui::PopFont();

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 sz = { EntryW, 5 + name_size.y + line1_size.y };

	const ImRect bb(pos, pos + sz);
	ItemSize(sz, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return UserAction::None;

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, 0);
	if (pressed)
		MarkItemEdited(id);

	// Render
	const ImU32 col = GetColorU32((held && hovered && !OverrideColor) ? ImGuiCol_ButtonActive : hovered && !OverrideColor ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	RenderNavHighlight(bb, id);
	RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	
	if (NXThemeHasPreview && (hovered || held) && gamepad.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER])
	{
		auto Preview = NXGetPreview();
		if (Preview)
		{
			ImGui::GetOverlayDrawList()->AddImage(
				(ImTextureID)Preview,
				{ 0,0 }, { SCR_W, SCR_H });
			return UserAction::Preview;
		}
	}

	ImGui::PushFont(font30);
	RenderText({ pos.x + 2, pos.y + 2 }, lblFname.c_str(), 0, false);
	ImGui::PopFont();
	ImGui::PushFont(font25);
	RenderText({ pos.x + EntryW - line2_size.x - 2, pos.y + 2 }, lblLine2.c_str(), 0, false);
	RenderTextWrapped({ pos.x + 2, pos.y + name_size.y + 2 }, lblLine1.c_str(), 0, EntryW - 5);
	ImGui::PopFont();
	
	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
	return pressed && Utils::ItemNotDragging() ? UserAction::Install : UserAction::None;
}

static bool PatchBG(SzsPatcher &Patcher, const vector<u8> &data, const string &SzsName)
{
	if (!Patcher.PatchMainBG(data))
	{
		Dialog("PatchBntx failed for " + SzsName + "\nThe theme was not installed");
		return false;
	}
	return true;
}

static bool PatchLayout(SzsPatcher& Patcher, const string &JSON, const string &PartName)
{
	auto patch = Patches::LoadLayout(JSON);
	if (!patch.IsCompatible(Patcher.GetSarc()))
	{
		Dialog("The provided layout is not compatible with " + PartName + "\nThe theme was not installed");
		return false;
	}
	Patcher.SetPatchAnimations(Settings::UseAnimations);
	if (!Patcher.PatchLayouts(patch, PartName, HOSVer.major >= 8 && PartName == "home"))
	{
		Dialog("PatchLayouts failed for " + PartName + "\nThe theme was not installed");
		return false;				
	}
	if (Settings::UseAnimations)
	{
		if (!Patcher.PatchAnimations(patch.Anims))
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

static inline bool SarcOpen(const string &path, SARC::SarcData *out)
{
	auto f = fs::OpenFile(path);
	if (f.size() == 0) return false;
	f = Yaz0::Decompress(f);
	*out = SARC::Unpack(f);
	return true;
}

static inline vector<u8> SarcPack(SARC::SarcData &data)
{
	auto packed = SARC::Pack(data);
	return Yaz0::Compress(packed.data, 3, packed.align);
}

static bool ExefsCompatAsk(const string& SzsName)
{
	if (!PatchMng::CanInstallTheme(SzsName))
		return YesNoPage::Ask(PatchMng::InstallWarnStr);
	return true;
}

//Uses blocking functions, only callable from Update()
bool ThemeEntry::InstallTheme(bool ShowLoading, const string& homeDirOverride)
{
	if (!CanInstall)
	{
		Dialog("Can't install this theme, check that it hasn't been corrupted and that you are using an updated version of this installer");
		return false;
	}
	try 
	{
		if (IsFont())
		{
			if (homeDirOverride != "")
			{
				DialogBlocking("Can't install a font to theme shuffle.");
				return false;
			}

			if (ShowLoading)
				DisplayLoading("Installing font...");

			fs::CreateFsMitmStructure("0100000000000811");
			fs::CreateRomfsDir("0100000000000811");
			fs::WriteFile(CfwFolder + "/titles/0100000000000811/romfs/nintendo_udsg-r_std_003.bfttf", SwitchThemesCommon::TTF::ConvertToBFTTF(file));
			fs::CreateFsMitmStructure("0100000000000039");
			fs::CreateRomfsDir("0100000000000039");
			fs::WriteFile(CfwFolder + "/titles/0100000000000039/romfs/dummy.bin", {0x70,0x61,0x70,0x65,0x20,0x53,0x61,0x74,0x61,0x6E,0x20,0x41,0x6C,0x65,0x70,0x70,0x65,0x21});
		}
		else if (LegacyTheme())
		{
			if (ShowLoading)
				DisplayLoading("Installing...");
			PatchTemplate patch = SwitchThemesCommon::SzsPatcher::DetectSarc(SData);

			if (!ExefsCompatAsk(patch.szsName))
					return false;

			fs::CreateThemeStructure(patch.TitleId);
			string szsPath;
			if (patch.TitleId == "0100000000001000" && homeDirOverride != "")
				szsPath = homeDirOverride + patch.szsName;
			else
				szsPath = CfwFolder + "/titles/" + patch.TitleId + "/romfs/lyt/" + patch.szsName;
			fs::WriteFile(szsPath, file);
		}
		else
		{
			auto themeInfo = ParseNXThemeFile(SData);
			
			if (!ExefsCompatAsk(ThemeTargetToFileName[themeInfo.Target]))
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

			if (ShowLoading)
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
					fs::WriteFile(CfwFolder + "/titles/0100000000001000/romfs/lyt/common.szs", SarcPack(Patcher.GetFinalSarc()));
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
					Dialog("Couldn't find any patch for " + BaseSzs + "\nThe theme was not installed");
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
									Dialog("Couldn't load the icon image for " + p.NxThemeName);
									continue;
								}
							}
							else continue;

							if (!pResult)
								Dialog(p.NxThemeName + " icon patch failed for " + SzsName + "\nThe theme will be installed anyway but may crash.");
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
							Dialog("Album icon patch failed for " + SzsName + "\nThe theme will be installed anyway but may crash.");
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
					fs::WriteFile(CfwFolder + "/titles/" + TitleId + "/romfs/lyt/" + SzsName, SarcPack(Patcher.GetFinalSarc()));
				}
			}
		}
	}
	catch (const exception& ex)
	{
		Dialog("Error while installing this theme: " + string(ex.what()));
		return false;
	}
FINISHED:
	if (ShowLoading)
		Dialog("Done, restart the console to see the changes");
	return true;
}