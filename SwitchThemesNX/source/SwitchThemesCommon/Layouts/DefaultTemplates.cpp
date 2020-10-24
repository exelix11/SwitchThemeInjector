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

using nlohmann::json;

#define get_s(s, f) j.at(s).get_to(f)
#define get(n) get_s(#n, p.n)

#define get_if_s(s, f) if(j.count(s)) j.at(s).get_to(f)
#define get_if(n) get_if_s(#n, p.n)

template<typename T>
static inline std::optional<T> GetOptionalHelper(const json& j, const char* name)
{
	if (j.count(name))
		return j.at(name).get<T>();
	return std::nullopt;
}

#define get_optional_s(s, f) f = GetOptionalHelper<decltype(f)::value_type>(j, s)
#define get_optional(n) get_optional_s(#n, p.n)

void from_json(const json& j, Vector2& p) {
	get(X);
	get(Y);
}

void from_json(const json& j, Vector3& p) {
	get(X);
	get(Y);
	get(Z);
}

void from_json(const json& j, UsdPatch& p) {
	p = {};
	get(PropName);
	get_if(PropValues);
	get_if(type);
}

void from_json(const json& j, PanePatch& p) {
	p = {};
	get(PaneName);

	#define get_assign_member(jname, field, flag) do { \
		if (j.count(jname)) { \
			j.at(jname).get_to(field); \
			p.ApplyFlags |= (u32)PanePatch::Flags::flag; \
		} } while(0)

	#define get_assign(name) get_assign_member(#name, p.name, name)
	
	get_assign(Position);
	get_assign(Rotation);
	get_assign(Scale);
	get_assign(Size);
	get_assign(Visible);

	get_assign(OriginX);
	get_assign(OriginY);
	get_assign(ParentOriginX);
	get_assign(ParentOriginY);

	get_assign_member("ColorTL", p.PaneSpecific0(), PaneSpecific0);
	get_assign_member("ColorTR", p.PaneSpecific1(), PaneSpecific1);
	get_assign_member("ColorBL", p.PaneSpecific2(), PaneSpecific2);
	get_assign_member("ColorBR", p.PaneSpecific3(), PaneSpecific3);

	get_assign(UsdPatches);

	#undef get_assign
	#undef get_assign_member
}

void from_json(const json& j, ExtraGroup& p) {
	p = {};	
	get(GroupName);
	get_if(Panes);
}

void from_json(const json& j, MaterialPatch::TexReference& p) {
	p = {};
	get(Name);
	get_optional(WrapS);
	get_optional(WrapT);
}

void from_json(const json& j, MaterialPatch::TexTransform& p) {
	p = {};
	get(Name);
	get_optional(X);
	get_optional(Y);
	get_optional(Rotation);
	get_optional(ScaleX);
	get_optional(ScaleY);
}

void from_json(const json& j, MaterialPatch& p) {
	p = {};
	get_if(MaterialName);
	get_if(ForegroundColor);
	get_if(BackgroundColor);
	get_if(Refs);
	get_if(Transforms);
}

void from_json(const json& j, LayoutFilePatch& p) {
	p = {};
	get_if(FileName);
	get_if(Patches);
	get_if(AddGroups);
	get_if(Materials);
	get_if(PushBackPanes);
	get_if(PullFrontPanes);
}

void from_json(const json& j, AnimFilePatch& p) {
	p = {};
	get_if(FileName);
	get_if(AnimJson);
}

void from_json(const json& j, LayoutPatch& p) {
	p = {};
	get_if(PatchName);
	get_if(AuthorName);
	get_if(Files);
	get_if(Anims);
	get_if(PatchAppletColorAttrib);
	get_if(ID);
	get_if(Obsolete_Ready8X);
}

#undef get_s
#undef get
#undef get_if_s
#undef get_if
#undef get_optional_s
#undef get_optional

LayoutPatch Patches::LoadLayout(const string_view jsn)
{
	return json::parse(jsn).get<LayoutPatch>();
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
		return json::parse(str).get<LayoutFilePatch>();
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