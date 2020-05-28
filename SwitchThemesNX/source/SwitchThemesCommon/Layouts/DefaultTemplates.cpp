#include "Patches.hpp"
#include "json.hpp"

using namespace std;

bool LayoutPatch::IsCompatible(const SARC::SarcData &sarc) const
{
	for (size_t i = 0; i < Files.size(); i++)
	{
		if (!sarc.files.count(Files[i].FileName)) return false;
		//For now this should be enough.
		/*string TargetFileAsString = ASCIIEncoding.ASCII.GetString(szs.Files[Files[i].FileName]);
		for (int j = 0; j < Files[i].Patches.Length; j++)
			if (!TargetFileAsString.Contains(Files[i].Patches[j].PaneName))
				return false;*/
	}
	return true;
}

using json = nlohmann::json;

#define ParseVec3(_n) {_n["X"],_n["Y"],_n["Z"]}
#define ParseVec2(_n) {_n["X"],_n["Y"]}
static LayoutFilePatch DeserializeFilePatch(const json &filePatch)
{
	if (!filePatch.count("FileName") || !filePatch.count("Patches"))
		return {""};
	LayoutFilePatch p;
	p.FileName = filePatch["FileName"].get<string>();
	for (auto panePatch : filePatch["Patches"])
	{
		if (!panePatch.count("PaneName"))
			continue;
		PanePatch pp;
		pp.PaneName = panePatch["PaneName"].get<string>();
		pp.ApplyFlags = 0;
		if (panePatch.count("Position"))
		{
			pp.ApplyFlags |= (u32)PanePatch::Flags::Position;
			pp.Position = ParseVec3(panePatch["Position"]);
		}if (panePatch.count("Rotation"))
		{
			pp.ApplyFlags |= (u32)PanePatch::Flags::Rotation;
			pp.Rotation = ParseVec3(panePatch["Rotation"]);
		}if (panePatch.count("Scale"))
		{
			pp.ApplyFlags |= (u32)PanePatch::Flags::Scale;
			pp.Scale = ParseVec2(panePatch["Scale"]);
		}if (panePatch.count("Size"))
		{
			pp.ApplyFlags |= (u32)PanePatch::Flags::Size;
			pp.Size = ParseVec2(panePatch["Size"]);
		}if (panePatch.count("Visible"))
		{
			pp.ApplyFlags |= (u32)PanePatch::Flags::Visible;
			pp.Visible = panePatch["Visible"].get<bool>();
		}

		if (panePatch.count("ColorTL"))
		{
			pp.ApplyFlags |= (u32)PanePatch::Flags::PaneSpecific0;
			pp.PaneSpecific0() = panePatch["ColorTL"].get<string>();
		}if (panePatch.count("ColorTR"))
		{
			pp.ApplyFlags |= (u32)PanePatch::Flags::PaneSpecific1;
			pp.PaneSpecific1() = panePatch["ColorTR"].get<string>();
		}if (panePatch.count("ColorBL"))
		{
			pp.ApplyFlags |= (u32)PanePatch::Flags::PaneSpecific2;
			pp.PaneSpecific2() = panePatch["ColorBL"].get<string>();
		}if (panePatch.count("ColorBR"))
		{
			pp.ApplyFlags |= (u32)PanePatch::Flags::PaneSpecific3;
			pp.PaneSpecific3() = panePatch["ColorBR"].get<string>();
		}

		if (panePatch.count("OriginX"))
		{
			pp.ApplyFlags |= (u32)PanePatch::Flags::OriginX;
			pp.OriginX = panePatch["OriginX"].get<u8>();
		}if (panePatch.count("OriginY"))
		{
			pp.ApplyFlags |= (u32)PanePatch::Flags::OriginY;
			pp.OriginY = panePatch["OriginY"].get<u8>();
		}if (panePatch.count("ParentOriginX"))
		{
			pp.ApplyFlags |= (u32)PanePatch::Flags::P_OriginX;
			pp.ParentOriginX = panePatch["ParentOriginX"].get<u8>();
		}if (panePatch.count("ParentOriginY"))
		{
			pp.ApplyFlags |= (u32)PanePatch::Flags::P_OriginY;
			pp.ParentOriginY = panePatch["ParentOriginY"].get<u8>();
		}

		if (panePatch.count("UsdPatches") && panePatch["UsdPatches"].is_array())
			for (auto& usdPatch : panePatch["UsdPatches"])
			{
				pp.UsdPatches.push_back({
					usdPatch["PropName"].get<string>(),
					usdPatch["PropValues"].get<vector<string>>(),
					usdPatch["type"].get<int>(),
					});
			}

		if (pp.UsdPatches.size() > 0)
			pp.ApplyFlags |= (u32)PanePatch::Flags::Usd1;

		p.Patches.push_back(pp);
	}
	if (filePatch.count("AddGroups") && filePatch["AddGroups"].is_array())
		for (auto& g : filePatch["AddGroups"])
		{
			if (!g.count("GroupName") || !g.count("Panes"))
				continue;
			ExtraGroup grp;
			grp.GroupName = g["GroupName"];
			grp.Panes = g["Panes"].get<vector<string>>();
			p.AddGroups.push_back(grp);
		}
	if (filePatch.count("Materials") && filePatch["Materials"].is_array())
		for (auto& m : filePatch["Materials"])
		{
			if (!m.count("MaterialName")) continue;
			MaterialPatch mat;
			mat.MaterialName = m["MaterialName"];
			mat.ForegroundColor = m.count("ForegroundColor") ? m["ForegroundColor"] : "";
			mat.BackgroundColor = m.count("BackgroundColor") ? m["BackgroundColor"] : "";
			p.Materials.push_back(mat);
		}
	if (filePatch.count("PushBackPanes"))
		p.PushBackPanes = filePatch["PushBackPanes"].get<vector<string>>();
	if (filePatch.count("PullFrontPanes"))
		p.PullFrontPanes = filePatch["PullFrontPanes"].get<vector<string>>();

	return p;
}

LayoutPatch Patches::LoadLayout(const string &jsn)
{
	LayoutPatch res;
	auto j = json::parse(jsn);
	if (j.count("PatchName"))
		res.PatchName = j["PatchName"].get<string>();
	if (j.count("AuthorName"))
		res.AuthorName = j["AuthorName"].get<string>();
	if (j.count("Ready8X"))
		res.Obsolete_Ready8X = j["Ready8X"].get<bool>();
	else res.Obsolete_Ready8X = false;
	if (j.count("ID"))
		res.ID = j["ID"].get<string>();
	else res.ID = "";

	if (j.count("PatchAppletColorAttrib"))
		res.PatchAppletColorAttrib = j["PatchAppletColorAttrib"].get<bool>();
	else res.PatchAppletColorAttrib = false;

	if (j.count("Anims") && j["Anims"].is_array())
	{
		for (auto& a : j["Anims"]) 
		{
			AnimFilePatch p;
			p.FileName = a["FileName"];
			p.AnimJson = a["AnimJson"];
			res.Anims.push_back(p);
		}
	}
	if (j.count("Files") && j["Files"].is_array())
	{
		for (auto& filePatch : j["Files"])
		{
			auto p = DeserializeFilePatch(filePatch);
			if (p.FileName != "")
				res.Files.push_back(p);
		}
	}
	return res;
}

//this is so ugly but c#-like aggregate initialization comes with c++20
//Whenever a new firmware breaks compatibility or layouts add a way to detect the new file here and increase the PatchRevision value, this is used later to fix old layouts via NewFirmFixes.cs
//Not using PatchRevision here because we can just figure out the firmware from the OS 
vector<PatchTemplate> Patches::DefaultTemplates{
//Common:
	PatchTemplate{ "home and applets", "common.szs", "0100000000001000", "<= 5.X",
        { "blyt/SystemAppletFader.bflyt" },
        { "blyt/DHdrSoft.bflyt" },
        "blyt/BgNml.bflyt",
        "White1x1_180^r",
        "exelixBG",
        { "P_Bg_00" },
        "White1x1^r" },
//Residentmenu:
	PatchTemplate{ "home menu", "ResidentMenu.szs", "0100000000001000", ">= 8.0",
		{ "blyt/IconError.bflyt", "blyt/RdtIconPromotion.bflyt" },
		{ "anim/RdtBtnShop_LimitB.bflan" },
		"blyt/BgNml.bflyt",
		"White1x1A128^s",
		"exelixBG",
		{ "P_Bg_00" },
		"White1x1A64^t" },
	PatchTemplate{ "home menu", "ResidentMenu.szs", "0100000000001000", ">= 6.0, < 8.0",
		{ "blyt/IconError.bflyt" },
		{ "anim/RdtBtnShop_LimitB.bflan" },
		"blyt/BgNml.bflyt",
		"White1x1A128^s",
		"exelixBG",
		{ "P_Bg_00" },
		"White1x1A64^t" },
	PatchTemplate{ "home menu only", "ResidentMenu.szs", "0100000000001000", "<= 5.X",
		{ "anim/RdtBtnShop_LimitB.bflan", "blyt/IconError.bflyt" },
		{},
		"blyt/RdtBase.bflyt",
		"White1x1A128^s",
		"exelixResBG",
		{ "L_BgNml" },
		"White1x1A64^t" },
//Entrance:
    PatchTemplate{ "lock screen", "Entrance.szs", "0100000000001000", "all firmwares",
        { "blyt/EntBtnResumeSystemApplet.bflyt" },
        {},
        "blyt/EntMain.bflyt",
        "White1x1^s",
        "exelixLK",
        { "P_BgL", "P_BgR" },
        "White1x1^r" },
//MyPage:
	PatchTemplate{ "user page", "MyPage.szs", "0100000000001013", "all firmwares",
        { "blyt/MypUserIconMini.bflyt", "blyt/BgNav_Root.bflyt" },
        {},
        "blyt/BgNml.bflyt",
        "NavBg_03^d",
        "exelixMY",
        { "P_Bg_00" },
        "White1x1A0^t" },
//Flaunch:
    PatchTemplate{ "all apps menu", "Flaunch.szs", "0100000000001000", ">= 6.X",
        { "blyt/FlcBtnIconGame.bflyt", "anim/BaseBg_Loading.bflan", "blyt/BgNav_Root.bflyt" }, //anim/BaseBg_Loading.bflan for 6.0
        {},
        "blyt/BgNml.bflyt",
        "NavBg_03^d",
        "exelixFBG",
        { "P_Bg_00" },
        "White1x1A64^t" },
//Set:
    PatchTemplate{ "settings applet", "Set.szs", "0100000000001000", ">= 6.X",
        { "blyt/BgNav_Root.bflyt", "blyt/SetCntDataMngPhoto.bflyt", "blyt/SetSideStory.bflyt" }, //blyt/SetSideStory.bflyt for 6.0 detection
        {},
        "blyt/BgNml.bflyt",
        "NavBg_03^d",
        "exelixSET",
        { "P_Bg_00" },
        "White1x1A0^t" },
//Notification:
    PatchTemplate{ "news applet", "Notification.szs", "0100000000001000", ">= 6.X",
        { "blyt/BgNavNoHeader.bflyt", "blyt/BgNav_Root.bflyt", "blyt/NtfBase.bflyt", "blyt/NtfImage.bflyt" }, //blyt/NtfImage.bflyt for 6.0
        {},
        "blyt/BgNml.bflyt",
        "NavBg_03^d",
        "exelixNEW",
        { "P_Bg_00" },
        "White1x1^r" },
//PSL:
    PatchTemplate{ "player selection", "Psl.szs", "0100000000001007", "all firmwares",
        { "blyt/IconGame.bflyt", "blyt/BgNavNoHeader.bflyt" },
        {},
        "blyt/PslSelectSinglePlayer.bflyt",
        "PselTopUserIcon_Bg^s",
        "exelixPSL",
        { "P_Bg" },
        "White1x1^r" },
};

namespace Patches::textureReplacement {


	//Do not manually edit, these are generated with the injector using TextureReplacement.GenerateJsonPatchesForInstaller()
	static constexpr string_view AlbumPatch	= "{\"FileName\":\"blyt/RdtBtnPvr.bflyt\",\"Patches\":[{\"PaneName\":\"P_Pict_00\",\"Position\":{\"X\":22.0,\"Y\":13.0,\"Z\":0.0},\"Size\":{\"X\":64.0,\"Y\":56.0},\"UsdPatches\":[{\"PropName\":\"C_W\",\"PropValues\":[\"100\",\"100\",\"100\",\"100\"],\"type\":1}]},{\"PaneName\":\"N_02\",\"Visible\":false},{\"PaneName\":\"N_01\",\"Visible\":false},{\"PaneName\":\"P_Pict_01\",\"Visible\":false},{\"PaneName\":\"P_Color\",\"Visible\":false}]}";
	static constexpr string_view NtfPatch	= "{\"FileName\":\"blyt/RdtBtnNtf.bflyt\",\"Patches\":[{\"PaneName\":\"P_PictNtf_00\",\"Size\":{\"X\":64.0,\"Y\":56.0},\"UsdPatches\":[{\"PropName\":\"C_W\",\"PropValues\":[\"100\",\"100\",\"100\",\"100\"],\"type\":1}]},{\"PaneName\":\"P_PictNtf_01\",\"Visible\":false}]}";
	static constexpr string_view ShopPatch	= "{\"FileName\":\"blyt/RdtBtnShop.bflyt\",\"Patches\":[{\"PaneName\":\"P_Pict\",\"Size\":{\"X\":64.0,\"Y\":56.0},\"UsdPatches\":[{\"PropName\":\"C_W\",\"PropValues\":[\"100\",\"100\",\"100\",\"100\"],\"type\":1}]}]}";
	static constexpr string_view CtrlPatch	= "{\"FileName\":\"blyt/RdtBtnCtrl.bflyt\",\"Patches\":[{\"PaneName\":\"P_Form\",\"Size\":{\"X\":64.0,\"Y\":56.0},\"UsdPatches\":[{\"PropName\":\"C_W\",\"PropValues\":[\"100\",\"100\",\"100\",\"100\"],\"type\":1}]},{\"PaneName\":\"P_Stick\",\"Visible\":false},{\"PaneName\":\"P_Y\",\"Visible\":false},{\"PaneName\":\"P_X\",\"Visible\":false},{\"PaneName\":\"P_A\",\"Visible\":false},{\"PaneName\":\"P_B\",\"Visible\":false}]}";
	static constexpr string_view SetPatch	= "{\"FileName\":\"blyt/RdtBtnSet.bflyt\",\"Patches\":[{\"PaneName\":\"P_Pict\",\"Size\":{\"X\":64.0,\"Y\":56.0},\"UsdPatches\":[{\"PropName\":\"C_W\",\"PropValues\":[\"100\",\"100\",\"100\",\"100\"],\"type\":1}]}]}";
	static constexpr string_view PowPatch	= "{\"FileName\":\"blyt/RdtBtnPow.bflyt\",\"Patches\":[{\"PaneName\":\"P_Pict_00\",\"Size\":{\"X\":64.0,\"Y\":56.0},\"UsdPatches\":[{\"PropName\":\"C_W\",\"PropValues\":[\"100\",\"100\",\"100\",\"100\"],\"type\":1}]}]}";
	static constexpr string_view LockPatch	= "{\"FileName\":\"blyt/EntBtnResumeSystemApplet.bflyt\",\"Patches\":[{\"PaneName\":\"P_PictHome\",\"Position\":{\"X\":0.0,\"Y\":0.0,\"Z\":0.0},\"Size\":{\"X\":184.0,\"Y\":168.0}}]}";

	static LayoutFilePatch GetPatch(const string_view &str)
	{
		return DeserializeFilePatch(json::parse(str));
	}

	static vector<TextureReplacement> ResidentMenu
	{
		{"album",     "RdtIcoPvr_00^s",   0x5050505, "blyt/RdtBtnPvr.bflyt",     "P_Pict_00",   64,56, GetPatch(AlbumPatch)	},
		{"news",      "RdtIcoNews_00^s",  0x5050505, "blyt/RdtBtnNtf.bflyt",     "P_PictNtf_00",64,56, GetPatch(NtfPatch)	},
		{"shop",      "RdtIcoShop^s",     0x5050505, "blyt/RdtBtnShop.bflyt",    "P_Pict",      64,56, GetPatch(ShopPatch)	},
		{"controller","RdtIcoCtrl_00^s",  0x5050505, "blyt/RdtBtnCtrl.bflyt",    "P_Form",		64,56, GetPatch(CtrlPatch)	},
		{"settings",  "RdtIcoSet^s",      0x5050505, "blyt/RdtBtnSet.bflyt",     "P_Pict",      64,56, GetPatch(SetPatch)	},
		{"power",     "RdtIcoPwrForm^s",  0x5050505, "blyt/RdtBtnPow.bflyt",     "P_Pict_00",   64,56, GetPatch(PowPatch)	},
	};

	static vector<TextureReplacement> Entrance
	{
		{"lock",     "EntIcoHome^s",  0x5040302, "blyt/EntBtnResumeSystemApplet.bflyt",  "P_PictHome", 184,168, GetPatch(LockPatch)}
	};
	
	unordered_map <string, vector<TextureReplacement>> NxNameToList
	{
		{"home", ResidentMenu},
		{"lock", Entrance}
	};
}