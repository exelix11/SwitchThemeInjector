#include "Patches.hpp"
#include "json.hpp"

using namespace std;

bool LayoutPatch::IsCompatible(const SARC::SarcData &sarc)
{
	for (int i = 0; i < Files.size(); i++)
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

LayoutPatch Patches::LoadLayout(const string &jsn)
{
#define ParseVec3(_n) {_n["X"],_n["Y"],_n["Z"]}
#define ParseVec2(_n) {_n["X"],_n["Y"]}

	LayoutPatch res;
	auto j = json::parse(jsn);
	if (j.count("PatchName"))
		res.PatchName = j["PatchName"].get<string>();
	if (j.count("AuthorName"))
		res.AuthorName = j["AuthorName"].get<string>();
	if (j.count("Ready8X"))
		res.Ready8X = j["Ready8X"].get<bool>();
	else res.Ready8X = false;

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
		for (auto &filePatch : j["Files"])
		{
			if (!filePatch.count("FileName") || !filePatch.count("Patches"))
				continue;
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
					pp.ApplyFlags |= (u32)PanePatch::Flags::ColorTL;
					pp.ColorTL = panePatch["ColorTL"].get<string>();
				}if (panePatch.count("ColorTR"))
				{
					pp.ApplyFlags |= (u32)PanePatch::Flags::ColorTR;
					pp.ColorTR = panePatch["ColorTR"].get<string>();
				}if (panePatch.count("ColorBL"))
				{
					pp.ApplyFlags |= (u32)PanePatch::Flags::ColorBL;
					pp.ColorBL = panePatch["ColorBL"].get<string>();
				}if (panePatch.count("ColorBR"))
				{
					pp.ApplyFlags |= (u32)PanePatch::Flags::ColorBR;
					pp.ColorBR = panePatch["ColorBR"].get<string>();
				}

				if (panePatch.count("UsdPatches") && panePatch["UsdPatches"].is_array())
					for (auto &usdPatch : panePatch["UsdPatches"])
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

			res.Files.push_back(p);
		}
	}
	return res;
}

//this is so ugly but c#-like aggregate initialization comes with c++20
vector<PatchTemplate> Patches::DefaultTemplates{
    PatchTemplate{ "home and applets", "common.szs", "0100000000001000", "<= 5.X",
        { "blyt/SystemAppletFader.bflyt" },
        { "blyt/DHdrSoft.bflyt" },
        "blyt/BgNml.bflyt",
        "White1x1_180^r",
        "exelixBG",
        { "P_Bg_00" },
        "White1x1^r" },
    PatchTemplate{ "home menu", "ResidentMenu.szs", "0100000000001000", ">= 6.X",
        { "blyt/IconError.bflyt" },
        { "anim/RdtBtnShop_LimitB.bflan" },
        "blyt/BgNml.bflyt",
        "White1x1A128^s",
        "exelixBG",
        { "P_Bg_00" },
        "White1x1A64^t" },
    PatchTemplate{ "lock screen", "Entrance.szs", "0100000000001000", "all firmwares",
        { "blyt/EntBtnResumeSystemApplet.bflyt" },
        {},
        "blyt/EntMain.bflyt",
        "White1x1^s",
        "exelixLK",
        { "P_BgL", "P_BgR" },
        "White1x1^r" },
    PatchTemplate{ "user page", "MyPage.szs", "0100000000001013", "all firmwares",
        { "blyt/MypUserIconMini.bflyt", "blyt/BgNav_Root.bflyt" },
        {},
        "blyt/BgNml.bflyt",
        "NavBg_03^d",
        "exelixMY",
        { "P_Bg_00" },
        "White1x1A0^t" },
    PatchTemplate{ "home menu only", "ResidentMenu.szs", "0100000000001000", "<= 5.X",
        { "anim/RdtBtnShop_LimitB.bflan", "blyt/IconError.bflyt" },
        {},
        "blyt/RdtBase.bflyt",
        "White1x1A128^s",
        "exelixResBG",
        { "L_BgNml" },
        "White1x1A64^t" },
    PatchTemplate{ "all apps menu", "Flaunch.szs", "0100000000001000", ">= 6.X",
        { "blyt/FlcBtnIconGame.bflyt", "anim/BaseBg_Loading.bflan", "blyt/BgNav_Root.bflyt" }, //anim/BaseBg_Loading.bflan for 6.0
        {},
        "blyt/BgNml.bflyt",
        "NavBg_03^d",
        "exelixFBG",
        { "P_Bg_00" },
        "White1x1A64^t" },
    PatchTemplate{ "settings applet", "Set.szs", "0100000000001000", ">= 6.X",
        { "blyt/BgNav_Root.bflyt", "blyt/SetCntDataMngPhoto.bflyt", "blyt/SetSideStory.bflyt" }, //blyt/SetSideStory.bflyt for 6.0 detection
        {},
        "blyt/BgNml.bflyt",
        "NavBg_03^d",
        "exelixSET",
        { "P_Bg_00" },
        "White1x1A0^t" },
    PatchTemplate{ "news applet", "Notification.szs", "0100000000001000", ">= 6.X",
        { "blyt/BgNavNoHeader.bflyt", "blyt/BgNav_Root.bflyt", "blyt/NtfBase.bflyt", "blyt/NtfImage.bflyt" }, //blyt/NtfImage.bflyt for 6.0
        {},
        "blyt/BgNml.bflyt",
        "NavBg_03^d",
        "exelixNEW",
        { "P_Bg_00" },
        "White1x1^r" },
    //PatchTemplate{ "options menu", "Option.szs", "0100000000001000", "all firmwares",
    //    { "blyt/OptMain.bflyt" },
    //    {},
    //    "blyt/BgPlate.bflyt",
    //    "NavBg_03^d",
    //    "exelixOP",
    //    { "P_PlateBg" },
    //    "White1x1^r" },
    PatchTemplate{ "player selection", "Psl.szs", "0100000000001007", "all firmwares",
        { "blyt/IconGame.bflyt", "blyt/BgNavNoHeader.bflyt" },
        {},
        "blyt/PslSelectSinglePlayer.bflyt",
        "PselTopUserIcon_Bg^s",
        "exelixPSL",
        { "P_Bg" },
        "White1x1^r" },
};