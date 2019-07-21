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
		lblFname = (GetFileName(FileName));
		lblLine1 = (FileName);
		lblLine2 = ("Folder");
		CanInstall = false;
		IsFolder = true;
	}
	else
	{
		file = OpenFile(FileName);
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
	lblLine1 = (GetFileName(FileName));
}

void ThemeEntry::ParseNxTheme()
{
	auto themeInfo = ParseNXThemeFile(SData);
	if (themeInfo.Version == -1)
	{
		lblLine1 = ("Invalid theme");
		CanInstall = false;
	}
	if (themeInfo.Version > 8)
	{
		lblLine2 = ("New version, update the installer !");
		CanInstall = false;
	}		
	if (CanInstall) {
		if (SData.files.count("image.dds"))
		{
			NXThemeHasPreview = true;
		}
		else if (SData.files.count("image.jpg"))
		{
			auto res = DDSConv::ImageToDDS(SData.files["image.jpg"], false, 1280, 720);
			if (res.size() != 0)
			{
				//HACK: don't save the nxtheme after this
				SData.files["image.dds"] = res;
				NXThemeHasPreview = true;
			}
			else 
			{
				CanInstall = false;
				lblLine2 = "Couldn't load the image";
			}
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

void ThemeEntry::ParseLegacyTheme()
{
	if (FileName == "")
	{
		lblFname = ("Unknown.szs");
		lblLine1 = ("Remote install");
	}
	else
	{
		lblFname = (GetFileName(FileName));
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
	auto Preview = ImageCache::LoadDDS(SData.files["image.dds"], FileName);
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
	
QUIT_RENDER:
	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
	return pressed && Utils::ItemNotDragging() ? UserAction::Install : UserAction::None;
}

static bool PatchBG(SzsPatcher &Patcher, const vector<u8> &data, const string &SzsName)
{
	auto pResult = Patcher.PatchMainBG(data);
	if (pResult != BflytFile::PatchResult::OK)
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
	auto res = Patcher.PatchLayouts(patch, PartName, NXTheme_FirmMajor >= 8 && PartName == "home");
	if (res != BflytFile::PatchResult::OK)
	{
		Dialog("PatchLayouts failed for " + PartName + "\nThe theme was not installed");
		return false;				
	}
	if (Settings::UseAnimations)
	{
		res = Patcher.PatchAnimations(patch.Anims);
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

//Uses blocking functions, only callable from Update()
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
			PatchTemplate patch = SwitchThemesCommon::SzsPatcher::DetectSarc(SData);
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
						
			//On 5.x some custom applet bg use common.szs
			bool DoPatchCommonBG = NXTheme_FirmMajor <= 5 && (themeInfo.Target == "news" || themeInfo.Target == "apps" || themeInfo.Target == "set");
			bool SkipSaveActualFile = false; //If the bg gets patched don't save the ResidentMenu file later
			if ((themeInfo.Target == "home" && SData.files.count("common.json")) && Settings::UseCommon || DoPatchCommonBG)
			{
				//common.szs patching code
				
				string CommonSzs = SD_PREFIX "/themes/systemData/common.szs";
				if (!filesystem::exists(CommonSzs))
				{
					MissingFileErrorDialog("common.szs");
					return false;
				}
				
				SzsPatcher Patcher(SarcOpen(CommonSzs));
				
				if (DoPatchCommonBG)
				{
					SkipSaveActualFile = true; //Do not save resident if the bg has been applied to common
					if (SData.files.count("image.dds"))
						if (!PatchBG(Patcher, SData.files["image.dds"], CommonSzs))
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
					WriteFile(homeDirOverride + "common.szs", SarcPack(Patcher.GetFinalSarc()));
				else 
				{
					CreateThemeStructure("0100000000001000");
					WriteFile(CfwFolder + "/titles/0100000000001000/romfs/lyt/common.szs", SarcPack(Patcher.GetFinalSarc()));
				}
			}
			
			SzsPatcher Patcher(SarcOpen(BaseSzs));
			string TitleId = "0100000000001000";
			string SzsName = ThemeTargetToFileName[themeInfo.Target];
			auto patch = Patcher.DetectSarc();
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
					if (!PatchBG(Patcher, SData.files["image.dds"], BaseSzs))
						return false;
			}
								
			if (SData.files.count("layout.json"))
			{
				SkipSaveActualFile = false;
				auto JsonBinary = SData.files["layout.json"];
				string JSON(reinterpret_cast<char*>(JsonBinary.data()), JsonBinary.size());
				if (!PatchLayout(Patcher, JSON, themeInfo.Target))
					return false;
			}

			if (Settings::UseIcons && Patches::textureReplacement::NxNameToList.count(themeInfo.Target))
			{
				auto& list = Patches::textureReplacement::NxNameToList[themeInfo.Target];
				for (const TextureReplacement& p : list)
				{
					auto pResult = BflytFile::PatchResult::Fail;
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

					if (pResult != BflytFile::PatchResult::OK)
						Dialog(p.NxThemeName + " icon patch failed for " + SzsName + "\nThe theme will be installed anyway but may crash.");
					else
						SkipSaveActualFile = false;
				}
			}
			
			if (!SkipSaveActualFile)
			{
				if (TitleId == "0100000000001000" && homeDirOverride != "")
					WriteFile(homeDirOverride + SzsName, SarcPack(Patcher.GetFinalSarc()));
				else {
					CreateThemeStructure(TitleId);
					WriteFile(CfwFolder + "/titles/" + TitleId + "/romfs/lyt/" + SzsName, SarcPack(Patcher.GetFinalSarc()));
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