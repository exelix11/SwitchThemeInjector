#include "SwitchThemesCommon.hpp"
#include "Bntx/QuickBntx.hpp"
#include "Bntx/DDS.hpp"
#include "Bntx/BRTI.hpp"
#include "Layouts/Bflan.hpp"
#include "NXTheme.hpp"
#include <algorithm>

using namespace std;
using namespace SwitchThemesCommon;

const string SwitchThemesCommon::CoreVer = "4.2 (C++)";

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
void SzsPatcher::SetPatchAnimations(bool enable) { EnableAnimations = enable; }

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

BflytFile::PatchResult SzsPatcher::PatchAnimations(const std::vector<AnimFilePatch>& files)
{
	u32 TargetVersion = 0;
	for (const auto& p : files)
	{
		if (!sarc.files.count(p.FileName))
			continue; //return BflytFile.PatchResult.Fail; Don't be so strict as older firmwares may not have all the animations (?)

		if (TargetVersion == 0)
		{
			auto bflan = new Bflan(sarc.files[p.FileName]);
			TargetVersion = bflan->Version;
			delete bflan;
		}

		auto bflan = BflanDeserializer::FromJson(p.AnimJson);
		bflan->Version = TargetVersion;
		bflan->byteOrder = Endianness::LittleEndian;
		sarc.files[p.FileName] = bflan->WriteFile();
		delete bflan;
	}
	return BflytFile::PatchResult::OK;
}

BflytFile::PatchResult SzsPatcher::PatchSingleLayout(const LayoutFilePatch& p)
{
	if (!sarc.files.count(p.FileName))
		return BflytFile::PatchResult::Fail;
	BflytFile target(sarc.files[p.FileName]);
	target.ApplyMaterialsPatch(p.Materials); //Ignore result for 8.0 less strict patching
	auto res = target.ApplyLayoutPatch(p.Patches);
	if (res != BflytFile::PatchResult::OK)
		return res;
	if (EnableAnimations)
	{
		res = target.AddGroupNames(p.AddGroups);
		if (res != BflytFile::PatchResult::OK)
			return res;
	}
	sarc.files[p.FileName] = target.SaveFile();
	return BflytFile::PatchResult::OK;
}

BflytFile::PatchResult SzsPatcher::PatchLayouts(const LayoutPatch& patch, const string &partName, bool Fix8x)
{
	if (partName == "home" && patch.PatchAppletColorAttrib)
		PatchBntxTextureAttribs({
			{"RdtIcoPvr_00^s",	0x2000000}, {"RdtIcoNews_00^s", 0x2000000},
			{"RdtIcoNews_01^s", 0x2000000}, {"RdtIcoSet^s",		0x2000000},
			{"RdtIcoShop^s",	0x2000000}, {"RdtIcoCtrl_00^s", 0x2000000},
			{"RdtIcoCtrl_01^s", 0x2000000}, {"RdtIcoCtrl_02^s", 0x2000000}, {"RdtIcoPwrForm^s", 0x2000000},
		});

	vector<LayoutFilePatch> Files;
	Files.insert(Files.end(), patch.Files.begin(), patch.Files.end());

	if (Fix8x && !patch.Ready8X)
	{
		auto extra = NewFirmFixes::GetFix(patch.PatchName);
		if (extra.size() != 0)
			Files.insert(Files.end(), extra.begin(), extra.end());
	}

	for (auto p : Files)
	{
		auto res = PatchSingleLayout(p);
		if (res != BflytFile::PatchResult::OK)
			return res;
	}
	return BflytFile::PatchResult::OK;
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

BflytFile::PatchResult SzsPatcher::PatchMainBG(const vector<u8> &DDS)
{
	PatchTemplate templ = DetectSarc();

	//Patch BG layouts
	BflytFile MainFile(sarc.files[templ.MainLayoutName]);
	auto res = MainFile.PatchBgLayout(templ);
	if (res == BflytFile::PatchResult::Fail || res == BflytFile::PatchResult::CorruptedFile) return res;
	
	sarc.files[templ.MainLayoutName] = MainFile.SaveFile();
	for (const auto& t : sarc.files)
	{
		auto& f = t.first;
		if (!StrEndsWith(f, ".bflyt") || !StrStartsWith(f, "blyt/") || f == templ.MainLayoutName) continue;
		BflytFile curTarget(sarc.files[f]);
		if (curTarget.PatchTextureName(templ.MaintextureName, templ.SecondaryTexReplace))
			sarc.files[f] = curTarget.SaveFile();
	}	

	//Patch bntx
	QuickBntx* q = OpenBntx();
	if (q->Rlt.size() != 0x80)
	{
		return BflytFile::PatchResult::CorruptedFile;
	}
	auto dds = DDSEncoder::LoadDDS(DDS);
	q->ReplaceTex(templ.MaintextureName, dds);

	return BflytFile::PatchResult::OK;
}

BflytFile::PatchResult SzsPatcher::PatchBntxTexture(const vector<u8> &DDS, const string &texName, u32 ChannelData)
{
	QuickBntx* q = OpenBntx();
	if (q->Rlt.size() != 0x80)
		return BflytFile::PatchResult::CorruptedFile;

	try
	{
		auto dds = DDSEncoder::LoadDDS(DDS);
		q->ReplaceTex(texName, dds);
		if (ChannelData != 0xFFFFFFFF)
			q->FindTex(texName)->ChannelTypes = ChannelData;
	}
	catch (...)
	{
		return BflytFile::PatchResult::Fail;
	}
	return BflytFile::PatchResult::OK;
}

BflytFile::PatchResult SzsPatcher::PatchAppletIcon(const std::vector<u8>& DDS, const std::string& texName)
{
	auto patch = DetectSarc();
	string nxthemeTarget = "";

	{
		auto it = find_if(ThemeTargetToFileName.begin(), ThemeTargetToFileName.end(),
			[&patch](const pair<string, string>& v) { return v.second == patch.szsName;	});
		if (it == ThemeTargetToFileName.end()) return BflytFile::PatchResult::Fail;
		nxthemeTarget = it->first;
	}

	if (!Patches::textureReplacement::NxNameToList.count(nxthemeTarget))
		return BflytFile::PatchResult::Fail;

	auto& list = Patches::textureReplacement::NxNameToList[nxthemeTarget];
	auto it = find_if(list.begin(), list.end(),
		[&texName](const TextureReplacement& t) {return t.NxThemeName == texName; });
	if (it == list.end()) 
		return BflytFile::PatchResult::Fail;

	auto res = PatchSingleLayout(it->patch);
	if (res != BflytFile::PatchResult::OK) return res;

	PatchBntxTexture(DDS, it->BntxName, it->NewColorFlags);

	BflytFile curTarget{ sarc.files[it->FileName] };
	curTarget.ClearUVData(it->PaneName);
	sarc.files[it->FileName] = curTarget.SaveFile();

	return BflytFile::PatchResult::OK;
}

BflytFile::PatchResult SzsPatcher::PatchBntxTextureAttribs(const vector<BntxTexAttribPatch> &patches)
{
	QuickBntx *q = OpenBntx();
	if (q->Rlt.size() != 0x80)
		return BflytFile::PatchResult::CorruptedFile;

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
		return BflytFile::PatchResult::Fail;
	}
	return BflytFile::PatchResult::OK;
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