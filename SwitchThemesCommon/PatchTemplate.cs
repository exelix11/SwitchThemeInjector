using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace SwitchThemes.Common
{
	/*
		The nxtheme format.
		nxtheme files are a yaz0-compressed sarc archives, aka szs files but the content is fully custom.
		A list of the allowed files follows, when a file has an alternative extension only one of the two can be in an archive:
			info.json - this is mandatory and it contains a serialized ThemeFileManifest struct.
				+ the Version field should be increased every time features are added
			image.dds/jpg - The main bg image
			layout.json - the main layout to be applied to the file, contains a serialized LayoutPatch struct
		*at least one of these two files must be in the theme for it to be valid*
		Home-menu only files: these files can only be present if the theme targets the home menu, all of them are optional.
			common.json - a layout to be applied to the common.szs file
			album.dds/png, news.dds/png, shop.dds/png, card.dds/png, controller.dds/png, nso.dds/png, settings.dds/png, share.dds/png, power.dds/png - custom applet icons
		Lock-screen only files:
			lock.dds/png - custom home icon

		non-DDS images are automatically converted by the installer to dds, as some algorithms may perform better than the built-in one the injector still uses dds images
		
		 Deprecated files:
			Preview.png - an image used for previewing the dds, not supported anymore as the installer now can load the dds directly
	*/

	public class ThemeFileManifest
	{
		public int Version;
		public string Author;
		public string ThemeName;
		public string LayoutInfo;
		public string Target;

		public string Serialize()
		{
			if (string.IsNullOrWhiteSpace(Target) || string.IsNullOrWhiteSpace(ThemeName))
				throw new Exception("Invalid target or theme name");
			JsonSerializerSettings settings = new JsonSerializerSettings()
			{
				DefaultValueHandling = DefaultValueHandling.Ignore,
				NullValueHandling = NullValueHandling.Ignore,
				Formatting = Formatting.None,
				ReferenceLoopHandling = ReferenceLoopHandling.Ignore,
			};
			return JsonConvert.SerializeObject(this, settings);
		}
		public static ThemeFileManifest Deserialize(string json) { return JsonConvert.DeserializeObject<ThemeFileManifest>(json); }
	}

	public class PatchTemplate
	{
		public string FirmName;
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

		public bool RequiresCodePatch = false;

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

			string json = JsonConvert.SerializeObject(DefaultTemplates.Templates, settings);
			System.IO.File.WriteAllText("DefaultTemplates.json", json);
		}
#endif
		public class ExtraTemplateResult 
		{
			public PatchTemplate[] Result;
			public Exception Exception;
        }
		
        // Returns null in case of errors
		public static ExtraTemplateResult LoadExtraTemplates()
		{
			if (System.IO.File.Exists("ExtraTemplates.json"))
			{
				try { 
					var res = JsonConvert.DeserializeObject<PatchTemplate[]>(System.IO.File.ReadAllText("ExtraTemplates.json"));
					return new ExtraTemplateResult { Result = res };
                }
                catch (Exception e)
                {
                    return new ExtraTemplateResult { Exception = e };
                }
            }
            return new ExtraTemplateResult { Result = new PatchTemplate[0] };
        }
#endif
	}

	public static class DefaultTemplates
	{
		public static PatchTemplate GetFor(SARCExt.SarcData sarc)
		{
			bool SzsHasKey(string key) => sarc.Files.ContainsKey(key);

			if (!SzsHasKey(@"timg/__Combined.bntx"))
				return null;

			var t = ExtraTemplates.Concat(Templates).Where(x =>
				x.FnameIdentifier.All(SzsHasKey) &&
				!x.FnameNotIdentifier.Any(SzsHasKey));

			return t.FirstOrDefault();
		}

		public static PatchTemplate[] ExtraTemplates = new PatchTemplate[0];

		public static readonly PatchTemplate[] Templates =
		{
		//Common:
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
		//Residentmenu:
			new PatchTemplate() { TemplateName = "home menu" , szsName = "ResidentMenu.szs", TitleId = "0100000000001000",  FirmName = ">= 6.0",
				FnameIdentifier = new string[] { @"blyt/IconError.bflyt" },
				FnameNotIdentifier = new string[] { @"anim/RdtBtnShop_LimitB.bflan" } ,
				MainLayoutName = @"blyt/BgNml.bflyt",
				MaintextureName = "White1x1A128^s",
				PatchIdentifier = "exelixBG",
				targetPanels = new string[] { "P_Bg_00" },
				SecondaryTexReplace = "White1x1A64^t",
				NXThemeName = "home"
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
		//Entrance:
			new PatchTemplate() { TemplateName = "lock screen" , szsName = "Entrance.szs", TitleId = "0100000000001000",  FirmName = ">= 9.0",
				FnameIdentifier = new string[] {  @"blyt/PageindicatorAlarm.bflyt", @"blyt/EntBtnResumeSystemApplet.bflyt"},
				FnameNotIdentifier = new string[] { } ,
				MainLayoutName =@"blyt/EntMain.bflyt",
				MaintextureName = "White1x1^s",
				PatchIdentifier = "exelixLK",
				targetPanels = new string[] { "P_BgL", "P_BgR" },
				SecondaryTexReplace ="White1x1^r",
				NXThemeName = "lock",
				RequiresCodePatch = true,
			},
			new PatchTemplate() { TemplateName = "lock screen" , szsName = "Entrance.szs", TitleId = "0100000000001000",  FirmName = "<= 8.0",
				FnameIdentifier = new string[] {  @"blyt/EntBtnResumeSystemApplet.bflyt"},
				FnameNotIdentifier = new string[] { @"blyt/PageindicatorAlarm.bflyt" } ,
				MainLayoutName =@"blyt/EntMain.bflyt",
				MaintextureName = "White1x1^s",
				PatchIdentifier = "exelixLK",
				targetPanels = new string[] { "P_BgL", "P_BgR" },
				SecondaryTexReplace ="White1x1^r",
				NXThemeName = "lock"
			},
		//MyPage:
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
		//Flaunch:
			new PatchTemplate() { TemplateName = "all apps menu" , szsName = "Flaunch.szs", TitleId = "0100000000001000", FirmName = ">= 6.0",
				FnameIdentifier = new string[] { @"blyt/FlcBtnIconGame.bflyt", @"anim/BaseBg_Loading.bflan", @"blyt/BgNav_Root.bflyt" }, //anim/BaseBg_Loading.bflan for 6.0
				FnameNotIdentifier = new string[] { } ,
				MainLayoutName = @"blyt/BgNml.bflyt",
				MaintextureName = "NavBg_03^d",
				PatchIdentifier = "exelixFBG",
				targetPanels = new string[] { "P_Bg_00" },
				SecondaryTexReplace = "White1x1^r",
				NXThemeName = "apps"
			},
		//Set:
			new PatchTemplate() { TemplateName = "settings applet" , szsName = "Set.szs", TitleId = "0100000000001000",  FirmName = ">= 6.0",
				FnameIdentifier = new string[] { @"blyt/BgNav_Root.bflyt", @"blyt/SetCntDataMngPhoto.bflyt" , @"blyt/SetSideStory.bflyt"}, //blyt/SetSideStory.bflyt for 6.0 detection
				FnameNotIdentifier = new string[] { } ,
				MainLayoutName = @"blyt/BgNml.bflyt",
				MaintextureName = "NavBg_03^d",
				PatchIdentifier = "exelixSET",
				targetPanels = new string[] { "P_Bg_00" },
				SecondaryTexReplace = "White1x1A0^t",
				NXThemeName = "set"
			},
		//Notification:
			new PatchTemplate() { TemplateName = "news applet" , szsName = "Notification.szs", TitleId = "0100000000001000", FirmName = ">= 6.0",
				FnameIdentifier = new string[] { @"blyt/BgNavNoHeader.bflyt", @"blyt/BgNav_Root.bflyt", @"blyt/NtfBase.bflyt", @"blyt/NtfImage.bflyt" }, //blyt/NtfImage.bflyt for 6.0
				FnameNotIdentifier = new string[] { } ,
				MainLayoutName = @"blyt/BgNml.bflyt",
				MaintextureName = "NavBg_03^d",
				PatchIdentifier = "exelixNEW",
				targetPanels = new string[] { "P_Bg_00" },
				SecondaryTexReplace = "White1x1^r",
				NXThemeName = "news"
			},
		//PSL:
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

	public class TextureReplacement
	{
		#region Patches

		/*
		 * The C_W property is weird, it seems to affect color in some way, it has 4 values but doesn't seem to argb or similar.
		 * the value [90,0,0,0] makes the eshop color.
		 * [100,100,100,100] is the value chosen for custom icons, it makes a white-ish color using dark theme and wrks fine for replacing the color,
		 * For some reason though the replaced color doesn't work as well when the white theme is enabled in the console settings, still not sure why.
		*/
		//Patches to center the applet buttons, only for the nxtheme builder
		readonly static LayoutFilePatch CtrlPatch = new LayoutFilePatch()
		{
			FileName = "blyt/RdtBtnCtrl.bflyt",
			Patches = new PanePatch[]
			{
				new PanePatch {
					PaneName = "P_Form", Size = new Vector2(64,56),
					UsdPatches = new List<UsdPatch>() { new UsdPatch() {PropName = "C_W", PropValues = new string[] { "100","100","100","100" }, type = 1 } } },
				new PanePatch { PaneName = "P_Stick", Visible = false },
				new PanePatch { PaneName = "P_Y", Visible = false },
				new PanePatch { PaneName = "P_X", Visible = false },
				new PanePatch { PaneName = "P_A", Visible = false },
				new PanePatch { PaneName = "P_B", Visible = false }
			}
		};

		readonly static LayoutFilePatch SetPatch = new LayoutFilePatch()
		{
			FileName = "blyt/RdtBtnSet.bflyt",
			Patches = new PanePatch[]
			{
				new PanePatch { PaneName = "P_Pict", Size = new Vector2(64,56),
					UsdPatches = new List<UsdPatch>() { new UsdPatch() {PropName = "C_W", PropValues = new string[] { "100","100","100","100" }, type = 1 } }}
			}
		};

		readonly static LayoutFilePatch ShopPatch = new LayoutFilePatch()
		{
			FileName = "blyt/RdtBtnShop.bflyt",
			Patches = new PanePatch[]
			{
				new PanePatch { PaneName = "P_Pict", Size = new Vector2(64,56),
				UsdPatches = new List<UsdPatch>() { new UsdPatch() {PropName = "C_W", PropValues = new string[] { "100","100","100","100" }, type = 1 } }}
			}
		};

		readonly static LayoutFilePatch PowPatch = new LayoutFilePatch()
		{
			FileName = "blyt/RdtBtnPow.bflyt",
			Patches = new PanePatch[]
			{
				new PanePatch { PaneName = "P_Pict_00", Size = new Vector2(64,56),
				UsdPatches = new List<UsdPatch>() { new UsdPatch() {PropName = "C_W", PropValues = new string[] { "100","100","100","100" }, type = 1 } }}
			}
		};

		readonly static LayoutFilePatch NtfPatch = new LayoutFilePatch()
		{
			FileName = "blyt/RdtBtnNtf.bflyt",
			Patches = new PanePatch[]
			{
				new PanePatch { PaneName = "P_PictNtf_00", Size = new Vector2(64,56),
				UsdPatches = new List<UsdPatch>() { new UsdPatch() {PropName = "C_W", PropValues = new string[] { "100","100","100","100" }, type = 1 } }},
				new PanePatch { PaneName = "P_PictNtf_01", Visible = false }
			}
		};

		readonly static LayoutFilePatch AlbumPatch = new LayoutFilePatch()
		{
			FileName = "blyt/RdtBtnPvr.bflyt",
			Patches = new PanePatch[]
			{
				/*Patch only one position value to allow layouts to move around the picture freely
				  new PanePatch { PaneName = "N_00", Position = new NullableVector3(0,0,0) },
				  Original values : N_00 is at -22; -13. P_Pict_00 is at 28;
				*/			
				new PanePatch { PaneName = "P_Pict_00", Size = new Vector2(64,56), Position = new Vector3(22,13,0),
					UsdPatches = new List<UsdPatch>() { new UsdPatch() {PropName = "C_W", PropValues = new string[] { "100","100","100","100" }, type = 1 } }},
				new PanePatch { PaneName = "N_02", Visible = false },
				new PanePatch { PaneName = "N_01", Visible = false },
				new PanePatch { PaneName = "P_Pict_01", Visible = false },
				new PanePatch { PaneName = "P_Color", Visible = false }
			}
		};

		readonly static LayoutFilePatch NsoPatch = new LayoutFilePatch()
		{
			FileName = "blyt/RdtBtnLR.bflyt",
			Patches = new PanePatch[]
			{
				new PanePatch { PaneName = "P_LR_00", Size = new Vector2(64,56)},
				new PanePatch { PaneName = "P_LR_01", Visible = false },
			}
		};

		readonly static LayoutFilePatch SplayPatch = new LayoutFilePatch()
		{
			FileName = "blyt/RdtBtnSplay.bflyt",
			Patches = new PanePatch[]
			{
				new PanePatch { PaneName = "P_Pict_00", Size = new Vector2(64,56),
				UsdPatches = new List<UsdPatch>() { new UsdPatch() {PropName = "C_W", PropValues = new string[] { "100","100","100","100" }, type = 1 } }},
				new PanePatch { PaneName = "N_Wave", Visible = false },
				new PanePatch { PaneName = "P_Pict_01", Visible = false },
				new PanePatch { PaneName = "P_Pict_02", Visible = false },
				new PanePatch { PaneName = "P_Pict_03", Visible = false }
			}
		};

		readonly static LayoutFilePatch VgcPatch = new LayoutFilePatch()
		{
			FileName = "blyt/RdtBtnVgc.bflyt",
			Patches = new PanePatch[]
			{
				new PanePatch { PaneName = "P_Pict_00", Size = new Vector2(64,56),
				UsdPatches = new List<UsdPatch>() { new UsdPatch() {PropName = "C_W", PropValues = new string[] { "100","100","100","100" }, type = 1 } }},
				new PanePatch { PaneName = "P_00", Visible = false },
				new PanePatch { PaneName = "P_01", Visible = false },
			}
		};

		readonly static LayoutFilePatch LockPatch = new LayoutFilePatch()
		{
			FileName = "blyt/EntBtnResumeSystemApplet.bflyt",
			Patches = new PanePatch[]
			{
				new PanePatch { PaneName = "P_PictHome", Size = new Vector2(184,168), Position = new Vector3(0,0,0) }
			}
		};
#endregion

		public string NxThemeName;
		public string[] BntxNames;
		public UInt32 NewColorFlags;
		public string FileName;
		public string PaneName;
		public int W, H;
		public LayoutFilePatch patch;

		public TextureReplacement(string name, string[] bntxNames, UInt32 cflag, string Fname, string Pname, int w, int h, LayoutFilePatch p)
		{
			NxThemeName = name;
			BntxNames = bntxNames;
			NewColorFlags = cflag;
			FileName = Fname;
			PaneName = Pname;
			W = w; H = h;
			patch = p;
		}

        // News texture name varies by version
		readonly static string[] newsTextures = new[] {"RdtIcoNews_00^s", "RdtIcoNews_00_Home^s"};

		readonly public static List<TextureReplacement> ResidentMenu = new List<TextureReplacement>
		{
			new TextureReplacement("album",     new[] {"RdtIcoPvr_00^s"},			0x5050505, "blyt/RdtBtnPvr.bflyt",		"P_Pict_00",		64,56, AlbumPatch),
			new TextureReplacement("news",      newsTextures,						0x5050505, "blyt/RdtBtnNtf.bflyt",		"P_PictNtf_00",		64,56, NtfPatch),
			new TextureReplacement("shop",      new[] {"RdtIcoShop^s"},				0x5050505, "blyt/RdtBtnShop.bflyt",		"P_Pict",			64,56, ShopPatch),
			new TextureReplacement("controller",new[] {"RdtIcoCtrl_00^s"},			0x5050505, "blyt/RdtBtnCtrl.bflyt",		"P_Form",			64,56, CtrlPatch),
			new TextureReplacement("settings",  new[] {"RdtIcoSet^s"},				0x5050505, "blyt/RdtBtnSet.bflyt",		"P_Pict",			64,56, SetPatch),
			new TextureReplacement("power",     new[] {"RdtIcoPwrForm^s"},			0x5050505, "blyt/RdtBtnPow.bflyt",		"P_Pict_00",		64,56, PowPatch),
			new TextureReplacement("nso",       new[] {"RdtIcoLR_00^s"},			0x5050505, "blyt/RdtBtnLR.bflyt",		"P_LR_00",			64,56, NsoPatch),
			new TextureReplacement("card",      new[] {"RdtIcoHomeVgc^s"},			0x5050505, "blyt/RdtBtnVgc.bflyt",		"P_Pict_00",		64,56, VgcPatch),
			new TextureReplacement("share",     new[] {"RdtIcoHomeSplayFrame^s"},	0x5050505, "blyt/RdtBtnSplay.bflyt",	"P_Pict_00",		64,56, SplayPatch),
		};

		readonly public static List<TextureReplacement> Entrance = new List<TextureReplacement>
		{
			new TextureReplacement("lock",     new[] {"EntIcoHome^s"}, 0x5040302, "blyt/EntBtnResumeSystemApplet.bflyt",     "P_PictHome",184,168, LockPatch),
		};

		readonly public static Dictionary<string, List<TextureReplacement>> NxNameToList = new Dictionary<string, List<TextureReplacement>>
		{
			{ "home", ResidentMenu},
			{ "lock", Entrance}
		};

#if WIN && DEBUG
		public static string GenerateJsonPatchesForInstaller()
		{
			JsonSerializerSettings settings = new JsonSerializerSettings()
			{
				DefaultValueHandling = DefaultValueHandling.Ignore,
				NullValueHandling = NullValueHandling.Ignore,
				Formatting = Formatting.None,
				ReferenceLoopHandling = ReferenceLoopHandling.Ignore
			};

			Dictionary<string, string> p = new Dictionary<string, string>();
			foreach (var v in NxNameToList.Values)
			{
				foreach (var i in v)
				{
					p.Add(i.NxThemeName, JsonConvert.SerializeObject(i.patch, settings));
				}
			}
			return JsonConvert.SerializeObject(p, settings);
		}
#endif
	}
}
