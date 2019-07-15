using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace SwitchThemes.Common
{
	public class ThemeFileManifest
	{
		public int Version;
		public string Author;
		public string ThemeName;
		public string LayoutInfo;
		public string Target;

		public string Serialize() { return JsonConvert.SerializeObject(this); }
		public static ThemeFileManifest Deserialize(string json) { return JsonConvert.DeserializeObject<ThemeFileManifest>(json); }
	}

	public class PatchTemplate
	{
		public string FirmName = "";
		public string TemplateName;
		public string szsName;
		public string TitleId;

		public string[] FnameIdentifier;
		public string[] FnameNotIdentifier;

		public string MainLayoutName;
		public string MaintextureName;
		public string PatchIdentifier;
		public string[] targetPanels;
		public string SecondaryTexReplace;

		public string NXThemeName;

		public bool DirectPatchPane = false;
		public bool NoRemovePanel = false;
		//public bool ReplaceTarget = false;
		//public PatchTemplate[] UnpatchTargets;

#if WIN
#if DEBUG
		public static void BuildTemplateFile()
		{
			JsonSerializerSettings settings = new JsonSerializerSettings()
			{
				Formatting = Formatting.Indented,
				DefaultValueHandling = DefaultValueHandling.Ignore,
				NullValueHandling = NullValueHandling.Ignore,
				ReferenceLoopHandling = ReferenceLoopHandling.Ignore,
			};

			string json = JsonConvert.SerializeObject(DefaultTemplates.templates, settings);
			System.IO.File.WriteAllText("DefaultTemplates.json", json);
		}
#endif
		public static PatchTemplate[] LoadTemplates() =>
			JsonConvert.DeserializeObject<PatchTemplate[]>(System.IO.File.ReadAllText("ExtraTemplates.json"));
#endif
	}

	public static class DefaultTemplates
	{
		public static readonly PatchTemplate[] templates =
		{
			new PatchTemplate() { TemplateName = "home and applets" , szsName = "common.szs", TitleId = "0100000000001000", FirmName = "<= 5.X",
				FnameIdentifier = new string[] { @"blyt/SystemAppletFader.bflyt"},
				FnameNotIdentifier = new string[] { @"blyt/DHdrSoft.bflyt" } ,
				MainLayoutName = @"blyt/BgNml.bflyt",
				MaintextureName = "White1x1_180^r",
				PatchIdentifier = "exelixBG",
				targetPanels = new string[] { "P_Bg_00" },
				SecondaryTexReplace = "White1x1^r",
				NXThemeName = "home",
			},
			new PatchTemplate() { TemplateName = "home menu" , szsName = "ResidentMenu.szs", TitleId = "0100000000001000",  FirmName = ">= 6.X",
				FnameIdentifier = new string[] { @"blyt/IconError.bflyt" },
				FnameNotIdentifier = new string[] { @"anim/RdtBtnShop_LimitB.bflan" } ,
				MainLayoutName = @"blyt/BgNml.bflyt",
				MaintextureName = "White1x1A128^s",
				PatchIdentifier = "exelixBG",
				targetPanels = new string[] { "P_Bg_00" },
				SecondaryTexReplace = "White1x1A64^t",
				NXThemeName = "home"
			},
			new PatchTemplate() { TemplateName = "lock screen" , szsName = "Entrance.szs", TitleId = "0100000000001000",  FirmName = "all firmwares",
				FnameIdentifier = new string[] {  @"blyt/EntBtnResumeSystemApplet.bflyt"},
				FnameNotIdentifier = new string[] { } ,
				MainLayoutName =@"blyt/EntMain.bflyt",
				MaintextureName = "White1x1^s",
				PatchIdentifier = "exelixLK",
				targetPanels = new string[] { "P_BgL", "P_BgR" },
				SecondaryTexReplace ="White1x1^r",
				NXThemeName = "lock"
			},
			new PatchTemplate() { TemplateName = "user page" , szsName = "MyPage.szs", TitleId = "0100000000001013",  FirmName = "all firmwares",
				FnameIdentifier = new string[] { @"blyt/MypUserIconMini.bflyt",@"blyt/BgNav_Root.bflyt" },
				FnameNotIdentifier = new string[] { } ,
				MainLayoutName = @"blyt/BgNml.bflyt",
				MaintextureName = "NavBg_03^d",
				PatchIdentifier = "exelixMY",
				targetPanels = new string[] { "P_Bg_00" },
				SecondaryTexReplace = "White1x1A0^t",
				NXThemeName = "user"
			},
			new PatchTemplate() { TemplateName = "home menu only" , szsName = "ResidentMenu.szs", TitleId = "0100000000001000",  FirmName = "<= 5.X",
				FnameIdentifier = new string[] { @"anim/RdtBtnShop_LimitB.bflan" ,@"blyt/IconError.bflyt"},
				FnameNotIdentifier = new string[] { } ,
				MainLayoutName = @"blyt/RdtBase.bflyt",
				MaintextureName = "White1x1A128^s",
				PatchIdentifier = "exelixResBG",
				targetPanels = new string[] { "L_BgNml" },
				SecondaryTexReplace = "White1x1A64^t",
				NXThemeName = "home"
			},
			new PatchTemplate() { TemplateName = "all apps menu" , szsName = "Flaunch.szs", TitleId = "0100000000001000", FirmName = ">= 6.X",
				FnameIdentifier = new string[] { @"blyt/FlcBtnIconGame.bflyt", @"anim/BaseBg_Loading.bflan", @"blyt/BgNav_Root.bflyt" }, //anim/BaseBg_Loading.bflan for 6.0
				FnameNotIdentifier = new string[] { } ,
				MainLayoutName = @"blyt/BgNml.bflyt",
				MaintextureName = "NavBg_03^d",
				PatchIdentifier = "exelixFBG",
				targetPanels = new string[] { "P_Bg_00" },
				SecondaryTexReplace = "White1x1A64^t",
				NXThemeName = "apps"
			},
			new PatchTemplate() { TemplateName = "settings applet" , szsName = "Set.szs", TitleId = "0100000000001000",  FirmName = ">= 6.X",
				FnameIdentifier = new string[] { @"blyt/BgNav_Root.bflyt", @"blyt/SetCntDataMngPhoto.bflyt" , @"blyt/SetSideStory.bflyt"}, //blyt/SetSideStory.bflyt for 6.0 detection
				FnameNotIdentifier = new string[] { } ,
				MainLayoutName = @"blyt/BgNml.bflyt",
				MaintextureName = "NavBg_03^d",
				PatchIdentifier = "exelixSET",
				targetPanels = new string[] { "P_Bg_00" },
				SecondaryTexReplace = "White1x1A0^t",
				NXThemeName = "set"
			},
			new PatchTemplate() { TemplateName = "news applet" , szsName = "Notification.szs", TitleId = "0100000000001000", FirmName = ">= 6.X",
				FnameIdentifier = new string[] { @"blyt/BgNavNoHeader.bflyt", @"blyt/BgNav_Root.bflyt", @"blyt/NtfBase.bflyt", @"blyt/NtfImage.bflyt" }, //blyt/NtfImage.bflyt for 6.0
				FnameNotIdentifier = new string[] { } ,
				MainLayoutName = @"blyt/BgNml.bflyt",
				MaintextureName = "NavBg_03^d",
				PatchIdentifier = "exelixNEW",
				targetPanels = new string[] { "P_Bg_00" },
				SecondaryTexReplace = "White1x1^r",
				NXThemeName = "news"
			},
			//new PatchTemplate() { TemplateName = "options menu" , szsName = "Option.szs", TitleId = "0100000000001000", FirmName = "all firmwares",
			//	FnameIdentifier = new string[] { "blyt/OptMain.bflyt" },
			//	FnameNotIdentifier = new string[] { } ,
			//	MainLayoutName = @"blyt/BgPlate.bflyt",
			//	MaintextureName = "NavBg_03^d",
			//	PatchIdentifier = "exelixOP",
			//	targetPanels = new string[] { "P_PlateBg" },
			//	SecondaryTexReplace = "White1x1^r",
			//	NXThemeName = "opt",
			//},
			new PatchTemplate() { TemplateName = "player selection" , szsName = "Psl.szs", TitleId = "0100000000001007", FirmName = "all firmwares",
				FnameIdentifier = new string[] { @"blyt/IconGame.bflyt", @"blyt/BgNavNoHeader.bflyt" },
				FnameNotIdentifier = new string[] { } ,
				MainLayoutName = @"blyt/PslSelectSinglePlayer.bflyt",
				MaintextureName = "PselTopUserIcon_Bg^s",
				PatchIdentifier = "exelixPSL",
				targetPanels = new string[] { "P_Bg" },
				SecondaryTexReplace = "White1x1^r",
				NXThemeName = "psl",
			},
		};
	}

	public struct AppletButtonPatch
	{
		public string NxThemeName;
		public string BntxName;
		public UInt32 NewColorFlags;

		public static List<AppletButtonPatch> Patches = new List<AppletButtonPatch>
		{
			new AppletButtonPatch() { NxThemeName = "album", BntxName = "RdtIcoPvr_00^s", NewColorFlags = 0x02000000 },
			new AppletButtonPatch() { NxThemeName = "news", BntxName = "RdtIcoNews_00^s", NewColorFlags = 0x02000000 },
			new AppletButtonPatch() { NxThemeName = "shop", BntxName = "RdtIcoShop^s", NewColorFlags = 0x02000000 },
			new AppletButtonPatch() { NxThemeName = "controller", BntxName = "RdtIcoCtrl_00^s", NewColorFlags = 0x02000000 },
			new AppletButtonPatch() { NxThemeName = "settings", BntxName = "RdtIcoSet^s", NewColorFlags = 0x02000000 },
			new AppletButtonPatch() { NxThemeName = "power", BntxName = "RdtIcoPwrForm^s", NewColorFlags = 0x02000000 },
		};
	}
}
