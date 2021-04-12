#include "SwitchThemesCommon.hpp"
#include "Bntx/QuickBntx.hpp"
#include "Bntx/DDS.hpp"
#include "Bntx/BRTI.hpp"
#include "Layouts/Bflan.hpp"
#include "NXTheme.hpp"
#include <algorithm>
#include "Layouts/Bflyt/Bflyt.hpp"
#include "Layouts/Bflyt/BflytPatcher.hpp"

using namespace std;
using namespace SwitchThemesCommon;

const string SwitchThemesCommon::CoreVer = "4.6.1 (C++)";
const int SwitchThemesCommon::NXThemeVer = 15;

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

SzsPatcher::SzsPatcher(SARC::SarcData&& s) : sarc(s) {}
SzsPatcher::SzsPatcher(SARC::SarcData& s) : sarc(s) {}

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

QuickBntx* SzsPatcher::OpenBntx() 
{
	if (bntx) return bntx;
	Buffer Reader(sarc.files["timg/__Combined.bntx"]);
	bntx = new QuickBntx(Reader);
	return bntx;
}

void SzsPatcher::SaveBntx()
{
	if (!bntx) return;
	sarc.files["timg/__Combined.bntx"] = bntx->Write();
	delete bntx;
	bntx = nullptr;
}

bool SzsPatcher::PatchSingleLayout(const LayoutFilePatch& p)
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

// TODO
bool SzsPatcher::PatchLayouts(const LayoutPatch& patch)
{
	auto szs = DetectSarc().szsName;

	auto t = std::find_if(ThemeTargetToFileName.begin(), ThemeTargetToFileName.end(), [&szs](const auto &e)
	{
		return e.second == szs;
	});

	return PatchLayouts(patch, t->first);
}

bool SzsPatcher::PatchLayouts(const LayoutPatch& patch, const string &partName)
{
	if (partName == "home" && patch.PatchAppletColorAttrib)
		PatchBntxTextureAttribs({
			{"RdtIcoPvr_00^s",	0x2000000}, {"RdtIcoNews_00^s", 0x2000000},
			{"RdtIcoNews_01^s", 0x2000000}, {"RdtIcoSet^s",		0x2000000},
			{"RdtIcoShop^s",	0x2000000}, {"RdtIcoCtrl_00^s", 0x2000000},
			{"RdtIcoCtrl_01^s", 0x2000000}, {"RdtIcoCtrl_02^s", 0x2000000}, {"RdtIcoPwrForm^s", 0x2000000},
		});

	vector<LayoutFilePatch> Files = patch.Files;

	if (patch.UsesOldFixes())
	{
		auto extra = NewFirmFixes::GetFixLegacy(patch.PatchName, partName);
		if (extra.size() != 0)
			Files.insert(Files.end(), extra.begin(), extra.end());
	}
	else if (patch.ID != "")
	{
		auto extra = NewFirmFixes::GetFix(patch.ID, partName);
		if (extra.size() != 0)
			Files.insert(Files.end(), extra.begin(), extra.end());
	}

	for (auto p : Files)
	{
		auto res = PatchSingleLayout(p);
		if (res != true)
			return res;
	}

	vector<AnimFilePatch> Anims = patch.Anims;

	vector<AnimFilePatch> extra;

	if (partName == "home") {
		if (patch.HideOnlineBtn)
			extra = NewFirmFixes::GetNoOnlineButtonFix();
		else if (std::none_of(Anims.begin(), Anims.end(), [](const auto& a) { return a.FileName == "anim/RdtBase_SystemAppletPos.bflan"; }))
			extra == NewFirmFixes::GetAppletsPositionFix();

		if (extra.size())
			Anims.insert(Anims.end(), extra.begin(), extra.end());
	}

	if (Anims.size())
	{
		auto bflan = new Bflan(sarc.files[Anims[0].FileName]);
		auto TargetVersion = bflan->Version;
		delete bflan;

		for (const auto& p : Anims)
		{
			if (!sarc.files.count(p.FileName))
				continue; //return BflytFile.PatchResult.Fail; Don't be so strict as older firmwares may not have all the animations (?)

			auto bflan = BflanDeserializer::FromJson(p.AnimJson);
			bflan->Version = TargetVersion;
			bflan->byteOrder = Endianness::LittleEndian;
			sarc.files[p.FileName] = bflan->WriteFile();
			delete bflan;
		}
	}

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
	PatchTemplate templ = DetectSarc();

	//Patch BG layouts
	BflytFile _MainFile(sarc.files[templ.MainLayoutName]);
	BflytPatcher MainFile(_MainFile);
	
	auto res = MainFile.PatchBgLayout(templ);
	if (!res) return res;
	
	sarc.files[templ.MainLayoutName] = _MainFile.SaveFile();
	for (const auto& t : sarc.files)
	{
		auto& f = t.first;
		if (!StrEndsWith(f, ".bflyt") || !StrStartsWith(f, "blyt/") || f == templ.MainLayoutName) continue;
		BflytFile _curTarget(sarc.files[f]);
		BflytPatcher curTarget(_curTarget);
		if (curTarget.PatchTextureName(templ.MaintextureName, templ.SecondaryTexReplace))
			sarc.files[f] = _curTarget.SaveFile();
	}	

	//Patch bntx
	QuickBntx* q = OpenBntx();
	if (q->Rlt.size() != 0x80)
	{
		return false;
	}
	auto dds = DDSEncoder::LoadDDS(DDS);
	q->ReplaceTex(templ.MaintextureName, dds);

	return true;
}

bool SzsPatcher::PatchBntxTexture(const vector<u8> &DDS, const string &texName, u32 ChannelData)
{
	QuickBntx* q = OpenBntx();
	if (q->Rlt.size() != 0x80)
		return false;

	try
	{
		auto dds = DDSEncoder::LoadDDS(DDS);
		q->ReplaceTex(texName, dds);
		if (ChannelData != 0xFFFFFFFF)
			q->FindTex(texName)->ChannelTypes = ChannelData;
	}
	catch (...)
	{
		return false;
	}
	return true;
}

bool SzsPatcher::PatchAppletIcon(const std::vector<u8>& DDS, const std::string& texName)
{
	auto patch = DetectSarc();
	string nxthemeTarget = "";

	{
		auto it = find_if(ThemeTargetToFileName.begin(), ThemeTargetToFileName.end(),
			[&patch](const pair<string, string>& v) { return v.second == patch.szsName;	});
		if (it == ThemeTargetToFileName.end()) return false;
		nxthemeTarget = it->first;
	}

	if (!Patches::textureReplacement::NxNameToList.count(nxthemeTarget))
		return false;

	auto& list = Patches::textureReplacement::NxNameToList[nxthemeTarget];
	auto it = find_if(list.begin(), list.end(),
		[&texName](const TextureReplacement& t) {return t.NxThemeName == texName; });
	if (it == list.end()) 
		return false;

	auto res = PatchSingleLayout(it->patch);
	if (!res) return res;

	PatchBntxTexture(DDS, it->BntxName, it->NewColorFlags);

	BflytFile _curTarget{ sarc.files[it->FileName] };
	BflytPatcher curTarget(_curTarget);

	curTarget.ClearUVData(it->PaneName);
	sarc.files[it->FileName] = _curTarget.SaveFile();

	return true;
}

bool SzsPatcher::PatchBntxTextureAttribs(const vector<BntxTexAttribPatch> &patches)
{
	QuickBntx *q = OpenBntx();
	if (q->Rlt.size() != 0x80)
		return false;

	try
	{
		for (const auto& patch : patches) 
		{
			auto tex = q->FindTex(patch.TargetTexutre);
			if (tex) tex->ChannelTypes = patch.ChannelData;
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}


PatchTemplate SzsPatcher::DetectSarc()
{
	return DetectSarc(sarc);
}

PatchTemplate SzsPatcher::DetectSarc(const SARC::SarcData& sarc)
{
#define SzsHasKey(_str) (sarc.files.count(_str))

	if (!SzsHasKey("timg/__Combined.bntx"))
		return {};

	for (auto p : Patches::DefaultTemplates)
	{
		if (!SzsHasKey(p.MainLayoutName))
			continue;
		bool isTarget = true;
		for(string s : p.FnameIdentifier)
		{
			if (!SzsHasKey(s))
			{
				isTarget = false;
				break;
			}
		}
		if (!isTarget) continue;
		for (string s : p.FnameNotIdentifier)
		{
			if (SzsHasKey(s))
			{
				isTarget = false;
				break;
			}
		}
		if (!isTarget) continue;
		return p;
	}
	return {};
#undef SzsHasKey
}