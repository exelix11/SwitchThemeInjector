using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace SwitchThemes.Common
{
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
		public string[] SecondaryLayouts;
		public Tuple<string, string> SecondaryTexReplace;

		public bool NoRemovePanel = false;
		//public bool ReplaceTarget = false;
		//public PatchTemplate[] UnpatchTargets;

#if DEBUG
#if WIN
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
#endif
		public static PatchTemplate[] LoadTemplates()=>
			JsonConvert.DeserializeObject<PatchTemplate[]>(System.IO.File.ReadAllText("ExtraTemplates.json"));
	}

	public static class DefaultTemplates
	{
		public static readonly PatchTemplate[] templates =
		{
			new PatchTemplate() { TemplateName = "home and applets" , szsName = "common.szs", TitleId = "0100000000001000", FirmName = "<= 5.X",
				FnameIdentifier = new string[] { },
				FnameNotIdentifier = new string[] { @"blyt/DHdrSoft.bflyt" } ,
				MainLayoutName = @"blyt/BgNml.bflyt",
				MaintextureName = "White1x1_180^r",
				PatchIdentifier = "exelixBG",
				targetPanels = new string[] { "P_Bg_00" },
				SecondaryLayouts = new string[] { @"blyt/SystemAppletFader.bflyt" },
				SecondaryTexReplace = new Tuple<string, string>("White1x1_180^r", "White1x1^r")
			},
			new PatchTemplate() { TemplateName = "home menu" , szsName = "ResidentMenu.szs", TitleId = "0100000000001000",  FirmName = "6.0",
				FnameIdentifier = new string[] { },
				FnameNotIdentifier = new string[] { @"anim/RdtBtnShop_LimitB.bflan" } ,
				MainLayoutName = @"blyt/BgNml.bflyt",
				MaintextureName = "White1x1A128^s",
				PatchIdentifier = "exelixBG",
				targetPanels = new string[] { "P_Bg_00" },
				SecondaryLayouts = new string[] { @"blyt/IconError.bflyt" },
				SecondaryTexReplace = new Tuple<string, string>("White1x1A128^s", "White1x1A64^t")
			},
			new PatchTemplate() { TemplateName = "lock screen" , szsName = "Entrance.szs", TitleId = "0100000000001000",  FirmName = "all firmwares",
				FnameIdentifier = new string[] { },
				FnameNotIdentifier = new string[] { } ,
				MainLayoutName =@"blyt/EntMain.bflyt",
				MaintextureName = "White1x1^s",
				PatchIdentifier = "exelixLK",
				targetPanels = new string[] { "P_BgL", "P_BgR" },
				SecondaryLayouts = new string[] { @"blyt/EntBtnResumeSystemApplet.bflyt"},
				SecondaryTexReplace = new Tuple<string, string>("White1x1^s", "White1x1^r")
			},
			new PatchTemplate() { TemplateName = "user page" , szsName = "MyPage.szs", TitleId = "0100000000001013",  FirmName = "all firmwares",
				FnameIdentifier = new string[] { @"blyt/MypUserIconMini.bflyt" },
				FnameNotIdentifier = new string[] { } ,
				MainLayoutName = @"blyt/BaseTop.bflyt",
				MaintextureName = "NavBg_03^d",
				PatchIdentifier = "exelixMY",
				targetPanels = new string[] { "L_AreaNav", "L_AreaMain" },
				SecondaryLayouts = new string[] { @"blyt/BgNav_Root.bflyt"},
				SecondaryTexReplace = new Tuple<string, string>("NavBg_03^d", "White1x1A0^t")
			},
			new PatchTemplate() { TemplateName = "home menu only" , szsName = "ResidentMenu.szs", TitleId = "0100000000001000",  FirmName = "<= 5.X",
				FnameIdentifier = new string[] { @"anim/RdtBtnShop_LimitB.bflan" },
				FnameNotIdentifier = new string[] { } ,
				MainLayoutName = @"blyt/RdtBase.bflyt",
				MaintextureName = "White1x1A128^s",
				PatchIdentifier = "exelixResBG",
				targetPanels = new string[] { "L_BgNml" },
				SecondaryLayouts = new string[] { @"blyt/IconError.bflyt" },
				SecondaryTexReplace = new Tuple<string, string>("White1x1A128^s", "White1x1A64^t")
			},


			new PatchTemplate() { TemplateName = "settings applet" , szsName = "Set.szs", TitleId = "0100000000001000",  FirmName = "6.0",
				FnameIdentifier = new string[] { @"blyt/SetCntDataMngPhoto.bflyt" , @"blyt/SetSideStory.bflyt"}, //blyt/SetSideStory.bflyt for 6.0 detection
				FnameNotIdentifier = new string[] { } ,
				MainLayoutName = @"blyt/BgNml.bflyt",
				MaintextureName = "White1x1A0^s",
				PatchIdentifier = "exelixSET",
				targetPanels = new string[] { "P_Bg_00" },
				SecondaryLayouts = new string[] { @"blyt/MeterDataMngSide.bflyt" , @"blyt/IllustMigUser.bflyt", @"blyt/TextH1Ex.bflyt"}, 
				SecondaryTexReplace = new Tuple<string, string>("White1x1A0^s", "White1x1A0^t")
			},
			new PatchTemplate() { TemplateName = "news applet" , szsName = "Notification.szs", TitleId = "0100000000001000", FirmName = "6.0",
				FnameIdentifier = new string[] { @"blyt/NtfBase.bflyt", @"blyt/NtfImage.bflyt" }, //blyt/NtfImage.bflyt for 6.0
				FnameNotIdentifier = new string[] { } ,
				MainLayoutName = @"blyt/BgNml.bflyt",
				MaintextureName = "White1x1A0^s",
				PatchIdentifier = "exelixNEW",
				targetPanels = new string[] { "P_Bg_00" },
				SecondaryLayouts = new string[] { @"blyt/NtfIconChannel.bflyt"},
				SecondaryTexReplace = new Tuple<string, string>("White1x1A0^s", "White1x1A0^t")
			},
			new PatchTemplate() { TemplateName = "all apps menu" , szsName = "FLaunch.szs", TitleId = "0100000000001000", FirmName = "6.0",
				FnameIdentifier = new string[] { @"blyt/FlcBtnIconGame.bflyt", @"anim/BaseBg_Loading.bflan" }, //anim/BaseBg_Loading.bflan for 6.0
				FnameNotIdentifier = new string[] { } ,
				MainLayoutName = @"blyt/BgNml.bflyt",
				MaintextureName = "NavBg_03^d",
				PatchIdentifier = "exelixFBG",
				targetPanels = new string[] { "P_Bg_00" },
				SecondaryLayouts = new string[] { @"blyt/BgNav_Root.bflyt"},
				SecondaryTexReplace = new Tuple<string, string>("NavBg_03^d", "White1x1A64^t")
			},
		};
	}
}
