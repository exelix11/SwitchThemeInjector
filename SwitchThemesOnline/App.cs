using Bridge;
using Bridge.Html5;
using ExtensionMethods;
using Newtonsoft.Json;
using SARCExt;
using SwitchThemes.Common;
using SwitchThemes.Common.Bntxx;
using System;
using System.IO;
using System.Linq;
using static SwitchThemes.Common.Bntxx.DDSEncoder;

namespace SwitchThemesOnline
{
	public class App
	{
		const float AppVersion = 1f;
		static HTMLDivElement loader = null;
		static HTMLParagraphElement LoaderText = null;
		static HTMLParagraphElement lblDetected = null;
		static HTMLParagraphElement lblTutorial = null;
		static HTMLParagraphElement lblDDSPath = null;
		static string DefaultTutorialText = "";

		static SarcData CommonSzs = null;
		static DDSLoadResult LoadedDDS = null;
		static PatchTemplate targetPatch = null;

		public static void OnLoad()
		{
#if DEBUG
			Document.GetElementById<HTMLDivElement>("DebugFlag").Hidden = false;
#endif
			Document.GetElementById<HTMLParagraphElement>("P_Version").TextContent = "Switch theme injector online - Version : " + AppVersion + " - Core version : " + SwitchThemesCommon.CoreVer;
			Document.GetElementById<HTMLDivElement>("D_JsWarn").Remove();
			string useragent = Window.Navigator.UserAgent.ToLower();
			if (useragent.Contains("msie") || useragent.Contains("trident"))
			{
				Document.GetElementById<HTMLDivElement>("D_IeWarn").Hidden = false;
			}
			
			loader = Document.GetElementById<HTMLDivElement>("loaderDiv");
			LoaderText = Document.GetElementById<HTMLParagraphElement>("LoadingText");
			lblDetected = Document.GetElementById<HTMLParagraphElement>("P_DetectedSZS");
			lblTutorial = Document.GetElementById<HTMLParagraphElement>("P_Tutorial");
			DefaultTutorialText = lblTutorial.InnerHTML;
			lblTutorial.InnerHTML = string.Format(DefaultTutorialText, "*szs name*", "*title id*").Replace("\r\n", "<br />");
			lblDDSPath = Document.GetElementById<HTMLParagraphElement>("P_DDSPath");

			Document.GetElementById<HTMLParagraphElement>("P_PatchList").InnerHTML = SwitchThemesCommon.GeneratePatchListString(DefaultTemplates.templates).Replace("\r\n", "<br />");

			LoadAutoThemeState();
		}

		public static void UploadSZS(Uint8Array arr) //called from js
		{
			DoActionWithloading(() => 
			{
				byte[] sarc = ManagedYaz0.Decompress(arr.ToArray());
				CommonSzs = SARCExt.SARC.UnpackRamN(sarc);
				sarc = null;
				targetPatch = SwitchThemesCommon.DetectSarc(CommonSzs, DefaultTemplates.templates);
				if (targetPatch == null)
				{
					Window.Alert("This is not a valid theme file, if it's from a newer firmware it's not compatible with this tool yet");
					CommonSzs = null;
					targetPatch = null;
					lblDetected.TextContent = "";
					return;
				}
				lblDetected.TextContent = "Detected " + targetPatch.TemplateName + " " + targetPatch.FirmName;
				lblTutorial.InnerHTML = string.Format(DefaultTutorialText, targetPatch.szsName, targetPatch.TitleId).Replace("\r\n", "<br />");
			});
		}

		public static void UploadDDSBtn() //called from button click
		{
			if (CommonSzs != null)
				Document.GetElementById<HTMLInputElement>("DdsUploader").Click();
			else
				Window.Alert("Open an szs first !");
		}

		public static void UploadDDS(Uint8Array arr, string fileName) //called from file uploader
		{
			DoActionWithloading(() => 
			{
				lblDDSPath.TextContent = fileName;
				LoadedDDS = DDSEncoder.LoadDDS(arr.ToArray());
			});
		}

		public static void PatchAndSave()
		{
			if (CommonSzs == null)
			{
				Window.Alert("Open an szs first !");
				return;
			}
			if (LoadedDDS == null)
			{
				Window.Alert("Open a DDS first !");
				return;
			}
			DoActionWithloading(() =>
			{
				if (SwitchThemesCommon.PatchBntx(CommonSzs, LoadedDDS, targetPatch) == BflytFile.PatchResult.Fail)
				{
					Window.Alert(
							"Can't build this theme: the szs you opened doesn't contain some information needed to patch the bntx," +
							"without this information it is not possible to rebuild the bntx." +
							"You should use an original or at least working szs");
					return;
				}
				
				var res = SwitchThemesCommon.PatchLayouts(CommonSzs, targetPatch);

				if (res == BflytFile.PatchResult.Fail)
				{
					Window.Alert("Couldn't patch this file, it might have been already modified or it's from an unsupported system version.");
					return;
				}
				else if (res == BflytFile.PatchResult.CorruptedFile)
				{
					Window.Alert("This file has been already patched with another tool and is not compatible, you should get an unmodified layout.");
					return;
				}
				var sarc = SARC.PackN(CommonSzs);
				byte[] yaz0 = ManagedYaz0.Compress(sarc.Item2, 1, (int)sarc.Item1);
				sarc = null;
				Uint8Array dwn = new Uint8Array(yaz0);
				string DownloadFname = targetPatch.szsName;
				Script.Write("downloadBlob(dwn,DownloadFname,'application/octet-stream');");
			});
		}

		static string AutoThemePartName = "";
		public static void AutoThemeUploadBtn(string PartName)
		{
			AutoThemePartName = PartName;
			Document.GetElementById<HTMLInputElement>("AutoThemeUploader").Click();
		}

		public readonly static string[] ValidAutoThemeParts = new string[] { "home", "lock", "user" };
		public static void AutoThemeFileUploaded(Uint8Array arr)
		{
			if (!ValidAutoThemeParts.ContainsStr(AutoThemePartName))
			{
				Window.Alert("An invalid part name has been selected");
				return;
			}
			DoActionWithloading(() =>
			{
				byte[] szs = arr.ToArray();
				byte[] sarc = ManagedYaz0.Decompress(szs);
				var szsData = SARCExt.SARC.UnpackRamN(sarc);
				sarc = null;
				var detected = SwitchThemesCommon.DetectSarc(szsData, DefaultTemplates.templates);
				if (detected == null)
				{
					Window.Alert("This is not a valid theme file, if it's from a newer firmware it's not compatible with this tool yet");
					return;
				}
				if (!detected.TemplateName.Contains(AutoThemePartName))
				{
					Window.Alert("This szs is valid but it doesn't look like the right one, you can keep it but it might generate themes that affect the wrong parts of the menu");
				}
				Window.LocalStorage.SetItem(AutoThemePartName, Convert.ToBase64String(szs));
				Window.LocalStorage.SetItem(AutoThemePartName + "Name", "Detected " + detected.TemplateName + " " + detected.FirmName);
				LoadAutoThemeState();				
			});
		}

		public static void AutoThemeDeleteAll()
		{
			if (!Window.Confirm("This will delete all the szs files stored in your browser. Auto-theme won't work anymore. Are you sure ?")) 
				return;
			foreach (var p in ValidAutoThemeParts)
			{
				Window.LocalStorage.RemoveItem(p);
				Window.LocalStorage.RemoveItem(p + "Name");
			}
			LoadAutoThemeState();
		}

		public static void LoadAutoThemeState()
		{
			foreach (var p in ValidAutoThemeParts)
			{
				string name = Window.LocalStorage.GetItem(p + "Name") as string;
				Document.GetElementById<HTMLParagraphElement>("P_" + p + "SZS").TextContent = name ?? "not uploaded yet";
			}
		}

		static void DoActionWithloading(Action action)
		{
			LoaderText.TextContent = loadingFaces[new Random().Next(0, loadingFaces.Length)];
			loader.Style.Display = "";
			Window.SetTimeout(() => { action(); loader.Style.Display = "none"; }, 100);
		}

		public static readonly string[] loadingFaces = new string[]
		{
			"(ﾉ≧∀≦)ﾉ・‥…━━━★","o͡͡͡╮༼ ಠДಠ ༽╭o͡͡͡━☆ﾟ.*･｡ﾟ",
			"༼∩✿ل͜✿༽⊃━☆ﾟ. * ･ ｡ﾟ","༼(∩ ͡°╭͜ʖ╮͡ ͡°)༽⊃━☆ﾟ. * ･ ｡ﾟ",
			"ᕦ( ✿ ⊙ ͜ʖ ⊙ ✿ )━☆ﾟ.*･｡ﾟ","(∩｀-´)⊃━☆ﾟ.*･｡ﾟ",
			"༼∩☉ل͜☉༽⊃━☆ﾟ. * ･ ｡ﾟ","╰( ͡° ͜ʖ ͡° )つ──☆*:・ﾟ",
			"(∩ ͡° ͜ʖ ͡°)⊃━☆ﾟ","੭•̀ω•́)੭̸*✩⁺˚",
			"(੭ˊ͈ ꒵ˋ͈)੭̸*✧⁺˚","✩°｡⋆⸜(ू｡•ω•｡)",
			"ヽ༼ຈل͜ຈ༽⊃─☆*:・ﾟ","╰(•̀ 3 •́)━☆ﾟ.*･｡ﾟ",
			"(*’▽’)ノ＾—==ΞΞΞ☆","(੭•̀ω•́)੭̸*✩⁺˚",
			"(っ・ω・）っ≡≡≡≡≡≡☆",". * ･ ｡ﾟ☆━੧༼ •́ ヮ •̀ ༽୨",
			"༼∩ •́ ヮ •̀ ༽⊃━☆ﾟ. * ･ ｡ﾟ","(⊃｡•́‿•̀｡)⊃━☆ﾟ.*･｡ﾟ",
			"★≡≡＼（`△´＼）","( ◔ ౪◔)⊃━☆ﾟ.*・",
			"彡ﾟ◉ω◉ )つー☆*","(☆_・)・‥…━━━★",
			"(つ◕౪◕)つ━☆ﾟ.*･｡ﾟ","(つ˵•́ω•̀˵)つ━☆ﾟ.*･｡ﾟ҉̛༽̨҉҉ﾉ",
			"✩°｡⋆⸜(ू˙꒳​˙ )","╰( ⁰ ਊ ⁰ )━☆ﾟ.*･｡ﾟ"
		};
	}
}