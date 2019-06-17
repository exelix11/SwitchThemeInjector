#include "SwitchThemesCommon.hpp"
#include "Bntx/QuickBntx.hpp"
#include "Bntx/DDS.hpp"
#include "Bntx/BRTI.hpp"
#include "Layouts/Bflan.hpp"
#include "NXTheme.hpp"

using namespace std;

const string SwitchThemesCommon::CoreVer = "4.1 (C++)";

string SwitchThemesCommon::GeneratePatchListString(const vector < PatchTemplate >& templates) 
{
	string curSection = "";
	string FileList = "";
	for (auto p : templates)
	{
		FileList += "["+ p.FirmName+"] "+ p.TemplateName +" : the file is called "+ p.szsName+" from title "+ p.TitleId +"\n";
	}
	return FileList;
}

BflytFile::PatchResult SwitchThemesCommon::PatchAnimations(SARC::SarcData& sarc, const std::vector<AnimFilePatch>& files)
{
	u32 TargetVersion = 0;
	for(const auto &p : files)
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

BflytFile::PatchResult SwitchThemesCommon::PatchLayouts(SARC::SarcData &sarc, const LayoutPatch& patch, const string &partName, bool Fix8x, bool AddAnimations)
{
	if (partName == "home" && patch.PatchAppletColorAttrib)
		SwitchThemesCommon::PatchBntxTextureAttribs(sarc, {
			{"RdtIcoPvr_00^s", 0x02000000}, {"RdtIcoNews_00^s", 0x02000000},
			{"RdtIcoNews_01^s", 0x02000000}, {"RdtIcoSet^s", 0x02000000},
			{"RdtIcoShop^s", 0x02000000}, {"RdtIcoCtrl_00^s", 0x02000000},
			{"RdtIcoCtrl_01^s", 0x02000000}, {"RdtIcoCtrl_02^s", 0x02000000}, {"RdtIcoPwrForm^s", 0x02000000},
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
		if (!sarc.files.count(p.FileName))
			return BflytFile::PatchResult::Fail;
		BflytFile target(sarc.files[p.FileName]);
		target.ApplyMaterialsPatch(p.Materials); //Ignore result for 8.0 less strict patching
		auto res = target.ApplyLayoutPatch(p.Patches);
		if (res != BflytFile::PatchResult::OK)
			return res;
		if (AddAnimations)
		{
			res = target.AddGroupNames(p.AddGroups);
			if (res != BflytFile::PatchResult::OK)
				return res;
		}
		sarc.files[p.FileName] = target.SaveFile();
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

BflytFile::PatchResult SwitchThemesCommon::PatchBgLayouts(SARC::SarcData &sarc, const PatchTemplate& templ) 
{
	BflytFile MainFile(sarc.files[templ.MainLayoutName]);
	auto res = MainFile.PatchBgLayout(templ);
	if (res == BflytFile::PatchResult::OK)
	{
		sarc.files[templ.MainLayoutName] = MainFile.SaveFile();
		for (const auto &f : sarc.names)
		{
			if (!StrEndsWith(f, ".bflyt") || !StrStartsWith(f, "blyt/") || f == templ.MainLayoutName) continue;
			BflytFile curTarget(sarc.files[f]);
			if (curTarget.PatchTextureName(templ.MaintextureName, templ.SecondaryTexReplace))
				sarc.files[f] = curTarget.SaveFile();
		}
	}
	return res;
}

BflytFile::PatchResult SwitchThemesCommon::PatchBntx(SARC::SarcData &sarc, const vector<u8> &DDS, const PatchTemplate &targetPatch) 
{
	Buffer Reader(sarc.files["timg/__Combined.bntx"]);
	QuickBntx q(Reader);
	if (q.Rlt.size() != 0x80)
	{
		return BflytFile::PatchResult::CorruptedFile;
	}
	auto dds = DDSEncoder::LoadDDS(DDS);
	q.ReplaceTex(targetPatch.MaintextureName, dds);
	sarc.files["timg/__Combined.bntx"] = q.Write();
	return BflytFile::PatchResult::OK;
}

BflytFile::PatchResult SwitchThemesCommon::PatchBntxTexture(SARC::SarcData &sarc, const vector<u8> &DDS, const string &texName, u32 ChannelData)
{
	Buffer Reader(sarc.files["timg/__Combined.bntx"]);
	QuickBntx q(Reader);
	if (q.Rlt.size() != 0x80)
		return BflytFile::PatchResult::CorruptedFile;

	try
	{
		auto dds = DDSEncoder::LoadDDS(DDS);
		q.ReplaceTex(texName, dds);
		if (ChannelData != 0xFFFFFFFF)
			q.FindTex(texName)->ChannelTypes = ChannelData;
		sarc.files["timg/__Combined.bntx"] = q.Write();
	}
	catch (...)
	{
		return BflytFile::PatchResult::Fail;
	}
	return BflytFile::PatchResult::OK;
}

BflytFile::PatchResult SwitchThemesCommon::PatchBntxTextureAttribs(SARC::SarcData &sarc, const vector<BntxTexAttribPatch> &patches)
{
	Buffer Reader(sarc.files["timg/__Combined.bntx"]);
	QuickBntx q(Reader);
	if (q.Rlt.size() != 0x80)
		return BflytFile::PatchResult::CorruptedFile;

	try
	{
		for (const auto& patch : patches) 
		{
			auto tex = q.FindTex(patch.TargetTexutre);
			if (tex) tex->ChannelTypes = patch.ChannelData;
		}
		sarc.files["timg/__Combined.bntx"] = q.Write();
	}
	catch (...)
	{
		return BflytFile::PatchResult::Fail;
	}
	return BflytFile::PatchResult::OK;
}

PatchTemplate SwitchThemesCommon::DetectSarc(const SARC::SarcData &sarc)
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