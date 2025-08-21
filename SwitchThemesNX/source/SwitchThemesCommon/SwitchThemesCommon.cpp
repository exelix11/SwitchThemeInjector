#include <algorithm>
#include <ranges>
#include <unordered_set>

#include "SwitchThemesCommon.hpp"
#include "Bntx/QuickBntx.hpp"
#include "Bntx/DDS.hpp"
#include "Bntx/BRTI.hpp"
#include "Layouts/Bflan.hpp"
#include "NXTheme.hpp"
#include "Layouts/Bflyt/Bflyt.hpp"
#include "Layouts/Bflyt/BflytPatcher.hpp"
#include "Layouts/LayoutCompatibility.hpp"

using namespace std;
using namespace SwitchThemesCommon;

// This is the C++ implementation of SwitchThemesCommon. While this is the one that runs on switch the main one to be used as a reference and for prototyping is the C# one.
// The C# version also has better comments on the rationale behind things like compatibility fixes and patch ordering.

const string SwitchThemesCommon::CoreVer = "4.8.2 (C++)";
const int SwitchThemesCommon::NXThemeVer = 16;

string SwitchThemesCommon::GeneratePatchListString(const vector<PatchTemplate>& templates) 
{
	string curSection = "";
	string FileList = "";
	for (auto p : templates)
	{
		FileList += "["+ p.FirmName+"] "+ p.TemplateName +" : the file is called "+ p.szsName+" from title "+ p.TitleId +"\n";
	}
	return FileList;
}

SzsPatcher::SzsPatcher(SARC::SarcData&& s) : sarc(s) { Initialize(); }
SzsPatcher::SzsPatcher(SARC::SarcData& s) : sarc(s) { Initialize(); }

void SzsPatcher::Initialize()
{
	currentFirmware = HOSVer.ToFirmwareEnum();
	
	currentTemplate = DetectSarc(sarc);
	
	if (!currentTemplate)
	{
		nxthemePartName = "";
	}
	else
	{
		// Note that the PatchTemplate class does not have an NxThemeName property here because it depends on the current firmware version, this is to support 5.0 and earlier.
		// Since that is ancient firmware it is not supported in the injector so it's a difference to keep in mind when reasoning about the mapping between the nxtheme name and the target szs name
		auto t = std::find_if(ThemeTargetToFileName.begin(), ThemeTargetToFileName.end(), [&](const auto& e)
			{
				return e.second == currentTemplate->szsName;
			});

		if (t == ThemeTargetToFileName.end())
			nxthemePartName = "";
		else
			nxthemePartName = t->first;
	}
}

SzsPatcher::~SzsPatcher()
{
	if (bntx)
		delete bntx;
}

const SARC::SarcData& SzsPatcher::GetSarc() { return sarc; }

SARC::SarcData& SzsPatcher::GetFinalSarc()
{
	SaveBntx();
	return sarc;
}

QuickBntx& SzsPatcher::OpenBntx() 
{
	if (bntx) return *bntx;
	Buffer Reader(sarc.files["timg/__Combined.bntx"]);
	bntx = new QuickBntx(Reader);
	return *bntx;
}

void SzsPatcher::SaveBntx()
{
	if (!bntx) return;
	sarc.files["timg/__Combined.bntx"] = bntx->Write();
	delete bntx;
	bntx = nullptr;
}

void SzsPatcher::ApplyRawPatch(const std::optional<LayoutPatch>& p)
{
	if (!p)
		return;

	const auto& patch = *p;
	ApplyRawPatch(&patch);
}

void SzsPatcher::ApplyRawPatch(const LayoutPatch* p)
{
	if (!p)
		return;

	for (auto& f : p->Files) ApplyLayoutPatch(f);
	for (auto& f : p->Anims) ApplyAnimPatch(f);
}

bool SzsPatcher::ApplyAnimPatch(const AnimFilePatch& p)
{
	if (!sarc.files.count(p.FileName))
		return false;

	if (!FirmwareTargetBflanVersion)
	{
		auto bflan = std::make_unique<Bflan>(sarc.files[p.FileName]);
		FirmwareTargetBflanVersion = bflan->Version;
	}

	auto bflan = BflanDeserializer::FromJson(p.AnimJson);
	bflan->Version = *FirmwareTargetBflanVersion;
	bflan->byteOrder = Endianness::LittleEndian;
	sarc.files[p.FileName] = bflan->WriteFile();

	return true;
}

bool SzsPatcher::ApplyLayoutPatch(const LayoutFilePatch& p)
{
	if (!sarc.files.count(p.FileName))
		return false;

	BflytFile _target(sarc.files[p.FileName]);
	BflytPatcher target(_target);
	target.ApplyMaterialsPatch(p.Materials); //Ignore result for 8.0 less strict patching
	auto res = target.ApplyLayoutPatch(p.Patches);
	if (res != true)
		return res;

	if (EnableAnimations)
	{
		res = target.AddGroupNames(p.AddGroups);
		if (res != true)
			return res;
	}

	for (const auto& n : p.PullFrontPanes)
		target.PanePullToFront(n);
	for (const auto& n : p.PushBackPanes)
		target.PanePushBack(n);

	sarc.files[p.FileName] = _target.SaveFile();
	return true;
}

bool SzsPatcher::PatchLayouts(const LayoutPatch& patch)
{
	return PatchLayouts(patch, nxthemePartName);
}

bool SzsPatcher::PatchLayouts()
{
	auto fakePatch = LayoutPatch{
		.PatchName = "stub"
	};

	return PatchLayouts(fakePatch, nxthemePartName);
}

int SzsPatcher::FilterIncompatibleAnimations(LayoutPatch& p)
{
	std::unordered_set<std::string> remove{};
	std::vector<Compatibility::CompatIssue> issues{};

	for (const auto& anim : p.Anims)
	{
		issues.clear();
		auto bflan = BflanDeserializer::FromJson(anim.AnimJson);
		Compatibility::CheckAnimationCompatibility(issues, p, sarc, anim.FileName, *bflan);

		// TODO: Should unknown files be treated as errors ?
		for (auto& issue : issues)
			if (issue.Severity == Compatibility::ProblemSeverity::Critical)
				remove.insert(anim.FileName);
	}

	std::erase_if(p.Anims, [&](const auto& e) {
		return remove.count(e.FileName);
	});

	return static_cast<int>(remove.size());
}

bool SzsPatcher::PatchLayouts(const LayoutPatch& original_patch, const string &partName)
{
	// We must make a copy of the patch because some fix function might modify it
	auto patch = original_patch;

	// Compatibility flags
	bool useLegacyFixes = false;
	bool useModernFixes = false;
	bool appletPositionFixes = false;
	bool onlineBtnFix = false;

	// Clear this when patching a new layout
	TotalNonCompatibleFixes = 0;

	if (CompatFixes == LayoutCompatibilityOption::Firmware10 && partName == "home")
		patch.HideOnlineBtn = true;

	if (CompatFixes == LayoutCompatibilityOption::Firmware11 && partName == "home")
	{
		patch.HideOnlineBtn = false;
		patch.TargetFirmware = static_cast<int>(ConsoleFirmware::Fw11_0);
	}

	if (CompatFixes != LayoutCompatibilityOption::DisableFixes) 
	{
		// Detect any compatibility patches we need
		useLegacyFixes = currentFirmware != ConsoleFirmware::Invariant && patch.UsesOldFixes();
		useModernFixes = !useLegacyFixes && patch.ID != "";
		appletPositionFixes = partName == "home" && NewFirmFixes::ShouldApplyAppletPositionFix(patch, currentFirmware);
		// The default for this on old layouts that don't specify it is true
		onlineBtnFix = partName == "home" && patch.HideOnlineBtn;
	}

	if (partName == "home" && patch.PatchAppletColorAttrib)
		PatchBntxTextureAttribs({
			{"RdtIcoPvr_00^s",       0x2000000}, 
			// Pre 20.0.0 news icon:
			{"RdtIcoNews_00^s",      0x2000000}, {"RdtIcoNews_01^s",      0x2000000},
			// 20.0.0 news icon:
			{"RdtIcoNews_00_Home^s", 0x2000000}, {"RdtIcoNews_01_Home^s", 0x2000000},
			{"RdtIcoSet^s",          0x2000000},
			{"RdtIcoShop^s",         0x2000000},
			{"RdtIcoCtrl_00^s",      0x2000000}, {"RdtIcoCtrl_01^s",      0x2000000}, {"RdtIcoCtrl_02^s",      0x2000000},
			{"RdtIcoPwrForm^s",      0x2000000},
		});

	// Apply patches. The order here matters.

	// First home menu fixes. These are applied early so later patches from the json can override them
	if (appletPositionFixes)
		ApplyRawPatch(NewFirmFixes::GetAppletsPositionFix(currentFirmware));

	if (onlineBtnFix)
		ApplyRawPatch(NewFirmFixes::GetLegacyAppletButtonsFix(currentFirmware));

	// GetFix might modify the layout to make it compatible.
	// So while its result must be applied as an overlay we must call it before applying the patch.
	std::optional<LayoutPatch> modern_fix = std::nullopt;
	if (useModernFixes)
		modern_fix = NewFirmFixes::GetFix(patch, currentFirmware);

	if (CompatFixes != LayoutCompatibilityOption::DisableFixes)
		TotalNonCompatibleFixes += FilterIncompatibleAnimations(patch);

	// Then json patches
	ApplyRawPatch(&patch);

	// Then fixes on top of known broken layouts
	if (useLegacyFixes)
		ApplyRawPatch(NewFirmFixes::GetFixLegacy(patch.PatchName, currentFirmware, partName));

	if (useModernFixes)
		ApplyRawPatch(modern_fix);

	return true;
}

static bool StrEndsWith(const std::string &str, const std::string &suffix)
{
	return str.size() >= suffix.size() &&
		str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

static bool StrStartsWith(const std::string &str, const std::string &prefix)
{
	return str.find(prefix, 0) == 0;
}

bool SzsPatcher::PatchMainBG(const vector<u8> &DDS)
{
	if (!currentTemplate)
		return false;

	auto& templ = *currentTemplate;

	//Patch BG layouts
	BflytFile _MainFile(sarc.files[templ.MainLayoutName]);
	BflytPatcher MainFile(_MainFile);
	
	auto res = MainFile.PatchBgLayout(templ);
	if (!res) return res;
	
	//Patch bntx
	QuickBntx& q = OpenBntx();
	if (q.Rlt.size() != 0x80)
		return false;

	auto dds = DDSEncoder::LoadDDS(DDS);
	q.ReplaceTex(templ.MaintextureName, dds);
	
	// Remove references to the texture we replaced from other layouts
	auto replaceWith = q.FindTex(templ.SecondaryTexReplace) ? templ.SecondaryTexReplace : "";
	
	if (replaceWith == "") {
		auto v = q.Textures | std::views::filter([](const auto& d) { return d.Name().starts_with("White"); });
		if (v.empty())
			return false;
		
		replaceWith = v.front().Name();
	}
	
	sarc.files[templ.MainLayoutName] = _MainFile.SaveFile();
	for (const auto& t : sarc.files)
	{
		auto& f = t.first;
		if (!StrEndsWith(f, ".bflyt") || !StrStartsWith(f, "blyt/") || f == templ.MainLayoutName) continue;
		BflytFile _curTarget(sarc.files[f]);
		BflytPatcher curTarget(_curTarget);
		if (curTarget.PatchTextureName(templ.MaintextureName, replaceWith))
			sarc.files[f] = _curTarget.SaveFile();
	}	

	return true;
}

bool SzsPatcher::PatchBntxTexture(const vector<u8> &DDS, const vector<string> &texNames, u32 ChannelData)
{
	QuickBntx& q = OpenBntx();
	if (q.Rlt.size() != 0x80)
		return false;

	try
	{

		for (const auto& texName : texNames)
		{
			auto tex = q.FindTex(texName);
			if(!tex) continue;

			auto dds = DDSEncoder::LoadDDS(DDS);
			q.ReplaceTex(texName, dds);
			if (ChannelData != 0xFFFFFFFF)
				q.FindTex(texName)->ChannelTypes = ChannelData;
			return true;
		}
	}
	catch (...)
	{
		return false;
	}
	return false;
}

bool SzsPatcher::PatchAppletIcon(const std::vector<u8>& DDS, const std::string& texName)
{
	if (nxthemePartName == "")
		return false;

	if (!Patches::textureReplacement::NxNameToList.count(nxthemePartName))
		return false;

	const auto& list = Patches::textureReplacement::NxNameToList[nxthemePartName];
	auto replacement = find_if(list.begin(), list.end(), [&texName](const TextureReplacement& t) {
		return t.NxThemeName == texName; 
	});

	if (replacement == list.end()) 
		return false;

	// If this is not the right firmware, skip it
	if (currentFirmware < replacement->MinFirmware)
		return true;

	auto res = ApplyLayoutPatch(replacement->patch);
	if (!res) return res;

	PatchBntxTexture(DDS, replacement->BntxNames, replacement->NewColorFlags);

	BflytFile _curTarget{ sarc.files[replacement->FileName] };
	BflytPatcher curTarget(_curTarget);

	curTarget.ClearUVData(replacement->PaneName);
	sarc.files[replacement->FileName] = _curTarget.SaveFile();

	return true;
}

bool SzsPatcher::PatchBntxTextureAttribs(const vector<BntxTexAttribPatch> &patches)
{
	QuickBntx& q = OpenBntx();
	if (q.Rlt.size() != 0x80)
		return false;

	try
	{
		for (const auto& patch : patches) 
		{
			auto tex = q.FindTex(patch.TargetTexutre);
			if (tex) tex->ChannelTypes = patch.ChannelData;
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}

const std::optional<PatchTemplate>& SzsPatcher::DetectedSarc()
{
	return currentTemplate;
}

std::optional<PatchTemplate> SzsPatcher::DetectSarc(const SARC::SarcData& sarc)
{
	if (!sarc.files.count("timg/__Combined.bntx"))
		return std::nullopt;

	for (auto p : Patches::DefaultTemplates)
	{
		if (!sarc.files.count(p.MainLayoutName))
			continue;

		bool isTarget = true;
		for (string s : p.FnameIdentifier)
		{
			if (!sarc.files.count(s))
			{
				isTarget = false;
				break;
			}
		}

		if (!isTarget) continue;

		for (string s : p.FnameNotIdentifier)
		{
			if (sarc.files.count(s))
			{
				isTarget = false;
				break;
			}
		}

		if (!isTarget) continue;

		return p;
	}

	return std::nullopt;
}