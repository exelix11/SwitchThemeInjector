#include "SwitchThemesCommon.hpp"
#include "Bntx/QuickBntx.hpp"
#include "Bntx/DDS.hpp"
#include "Bntx/BRTI.hpp"
#include "Layouts/Bflan.hpp"

using namespace std;

const string SwitchThemesCommon::CoreVer = "3.9 (C++)";

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
	for(const auto &p : files)
	{
		if (!sarc.files.count(p.FileName))
			continue; //return BflytFile.PatchResult.Fail; Don't be so strict as older firmwares may not have all the animations (?)
		sarc.files[p.FileName] = BflanDeserializer::FromJson(p.AnimJson).WriteFile();
	}
	return BflytFile::PatchResult::OK;
}

BflytFile::PatchResult SwitchThemesCommon::PatchLayouts(SARC::SarcData &sarc, const vector<LayoutFilePatch>& layouts, bool AddAnimations)
{
	for (auto p : layouts)
	{
		if (!sarc.files.count(p.FileName))
			return BflytFile::PatchResult::Fail;
		BflytFile target(sarc.files[p.FileName]);
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
		return BflytFile::PatchResult::Fail;
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
	{
		return BflytFile::PatchResult::Fail;
	}
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