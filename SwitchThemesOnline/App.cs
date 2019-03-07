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
		const string AppVersion = "2.2";
		static HTMLDivElement topError;
		static HTMLDivElement loader = null;
		static HTMLParagraphElement LoaderText = null;
		static HTMLParagraphElement lblDetected = null;
		static HTMLParagraphElement lblDDSPath = null;
		static HTMLParagraphElement lblDDSPath_NX = null;
		static HTMLSelectElement LayoutsComboBox = null;
		static HTMLDivElement LayoutPrevDiv = null;
		static HTMLImageElement LayoutPrevImg = null;

		static HTMLSelectElement HomePartBox_NX = null;
		static HTMLSelectElement LayoutsComboBox_NX = null;
		static HTMLDivElement LayoutPrevDiv_NX = null;
		static HTMLImageElement LayoutPrevImg_NX = null;

		static SarcData CommonSzs = null;
		static byte[] LoadedDDS = null;
		static PatchTemplate targetPatch = null;

		public readonly static string[] embedLyouts = new string[] { "SuchHm", "SuchLk" , "ZnHm", "GleLk" };
		public static LayoutPatch[] layoutPatches;

		public static void OnLoad()
		{
#if DEBUG
			Document.GetElementById<HTMLDivElement>("DebugFlag").Hidden = false;
#endif
			topError = Document.GetElementById<HTMLDivElement>("D_JsWarn");
			topError.Hidden = true;
			Document.GetElementById<HTMLParagraphElement>("P_Version").TextContent = "Switch theme injector online - Version : " + AppVersion + " - Core version : " + SwitchThemesCommon.CoreVer;
			string useragent = Window.Navigator.UserAgent.ToLower();
			if (useragent.Contains("msie") || useragent.Contains("trident"))
			{
				Document.GetElementById<HTMLDivElement>("D_IeWarn").Hidden = false;
			}
			
			loader = Document.GetElementById<HTMLDivElement>("loaderDiv");
			LoaderText = Document.GetElementById<HTMLParagraphElement>("LoadingText");
			lblDetected = Document.GetElementById<HTMLParagraphElement>("P_DetectedSZS");
			lblDDSPath = Document.GetElementById<HTMLParagraphElement>("P_DDSPath");
			lblDDSPath_NX = Document.GetElementById<HTMLParagraphElement>("P_DDSPath2");
			LayoutsComboBox = Document.GetElementById<HTMLSelectElement>("LayoutsBox");
			LayoutPrevDiv = Document.GetElementById<HTMLDivElement>("PreviewDiv");
			LayoutPrevImg = Document.GetElementById<HTMLImageElement>("PreviewImg");

			HomePartBox_NX = Document.GetElementById<HTMLSelectElement>("HomePartBox");
			LayoutsComboBox_NX = Document.GetElementById<HTMLSelectElement>("LayoutsBox2");
			LayoutPrevDiv_NX = Document.GetElementById<HTMLDivElement>("PreviewDiv2");
			LayoutPrevImg_NX = Document.GetElementById<HTMLImageElement>("PreviewImg2");

			Document.GetElementById<HTMLParagraphElement>("P_PatchList").InnerHTML = SwitchThemesCommon.GeneratePatchListString(DefaultTemplates.templates).Replace("\r\n", "<br />");

			LoadCustomLayouts();
		}
		
		public static void LoadCustomLayouts()
		{
			layoutPatches = new LayoutPatch[embedLyouts.Length];
				for (int i = 0; i < layoutPatches.Length; i++)
					GetLayoutPart(i);
		}

		static void PrintError(string err)
		{
			topError.TextContent = err;
			topError.Hidden = false;
		}

		static void GetLayoutPart(int i)
		{
			XMLHttpRequest req = new XMLHttpRequest();
			req.ResponseType = XMLHttpRequestResponseType.String;
			req.OnReadyStateChange = () =>
			{
				if (req.ReadyState != AjaxReadyState.Done) return;
				string DownloadRes = req.Response as string;
				if (DownloadRes == null || DownloadRes.Length == 0)
				{
					PrintError("Error downloading one of the embedded layouts : " + embedLyouts[i] + ", it won't be available to make themes");
					return;
				}
				layoutPatches[i] = LayoutPatch.LoadTemplate(DownloadRes);
				DownloadRes = null;
			};
			req.Open("GET", "layouts/" + embedLyouts[i] + ".json", true);
			req.Send();
		}

		public static void UploadSZS(Uint8Array arr) //called from js
		{
			DoActionWithloading(() => 
			{
				byte[] sarc = ManagedYaz0.Decompress(arr.ToArray());
				CommonSzs = SARCExt.SARC.UnpackRamN(sarc);
				sarc = null;
				while (LayoutsComboBox.LastChild.TextContent != "Don't patch")
					LayoutsComboBox.RemoveChild(LayoutsComboBox.LastChild);
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
				for (int i = 0; i < layoutPatches.Length; i++)
					if (layoutPatches[i] != null && layoutPatches[i].IsCompatible(CommonSzs))
						LayoutsComboBox.Add(new HTMLOptionElement() { TextContent = layoutPatches[i].ToString(), Value = i.ToString() });
			});
		}

		public static void UploadDDSBtn() //called from button click
		{
			Document.GetElementById<HTMLInputElement>("DdsUploader").Click();
		}

		public static void UploadDDS(Uint8Array arr, string fileName) //called from file uploader
		{
			DoActionWithloading(() =>
			{
				lblDDSPath.TextContent = fileName;
				lblDDSPath_NX.TextContent = fileName;
				LoadedDDS = arr.ToArray();
			});
		}

		public static void HomePartBoxOnChange()
		{
			if (HomePartBox_NX.SelectedIndex <= 0)
				return;

			while (LayoutsComboBox_NX.LastChild.TextContent != "Don't patch")
				LayoutsComboBox_NX.RemoveChild(LayoutsComboBox_NX.LastChild);
			for (int i = 0; i < layoutPatches.Length; i++)
				if (layoutPatches[i] != null && (layoutPatches[i].TargetName == null ||
					layoutPatches[i].TargetName.Contains(SwitchThemesCommon.PartToFileName[HomePartBox_NX.Value])))
					LayoutsComboBox_NX.Add(new HTMLOptionElement() { TextContent = layoutPatches[i].ToString(), Value = i.ToString() });
		}

		public static void LayoutBoxNXOnChange()
		{
			if (LayoutsComboBox_NX.SelectedIndex <= 0)
				LayoutPrevDiv_NX.Hidden = true;
			else
			{
				int index = int.Parse(((HTMLOptionElement)LayoutsComboBox_NX.Children[LayoutsComboBox_NX.SelectedIndex]).Value);
				LayoutPrevImg_NX.Src = "layouts/" + embedLyouts[index] + ".jpg";
				LayoutPrevImg_NX.Hidden = false;
			}
		}

		public static void LayoutBoxOnChange()
		{
			if (LayoutsComboBox.SelectedIndex <= 0)
				LayoutPrevDiv.Hidden = true;
			else
			{
				int index = int.Parse(((HTMLOptionElement)LayoutsComboBox.Children[LayoutsComboBox.SelectedIndex]).Value);
				LayoutPrevImg.Src = "layouts/" + embedLyouts[index] + ".jpg";
				LayoutPrevDiv.Hidden = false;
			}
		}

		static void MakeNxTheme(string partName, string name, string author, LayoutPatch targetLayout)
		{
			if (LoadedDDS == null)
			{
				Window.Alert("Open a DDS first !");
				return;
			}
			if (!ValidAutoThemeParts.Contains(partName))
			{
				Window.Alert("select a valid Home menu part");
				return;
			}
			if (name.Trim() == "")
			{
				Window.Alert("Enter a valid name");
				return;
			}
			DoActionWithloading(() =>
			{
				var dds = DDSEncoder.LoadDDS(LoadedDDS); //this will crash if the dds type is wrong

				var meta = new ThemeFileManifest()
				{
					Version = 1,
					Author = author,
					LayoutInfo = targetLayout != null ? targetLayout.ToString() : "",
					ThemeName = name,
					Target = partName,
				};

				var res = SwitchThemesCommon.GenerateNXTheme(meta, LoadedDDS, targetLayout == null ? null : System.Text.Encoding.UTF8.GetBytes(targetLayout.AsJson()));
				Uint8Array dwn = new Uint8Array(res);
				string DownloadFname = name + ".nxtheme";
				Script.Write("downloadBlob(dwn,DownloadFname,'application/octet-stream');");
			});
		}

		public static void BuildNxThemeNX()
		{
			LayoutPatch targetLayout = null;
			if (LayoutsComboBox_NX.SelectedIndex > 0)
				targetLayout = layoutPatches[int.Parse(
					((HTMLOptionElement)LayoutsComboBox_NX.Children[LayoutsComboBox_NX.SelectedIndex]).Value)];

			MakeNxTheme(HomePartBox_NX.Value,
				Document.GetElementById<HTMLInputElement>("NXname").Value,
				Document.GetElementById<HTMLInputElement>("NXauthor").Value,
				targetLayout);
		}

		public static void BuildNxTheme()
		{
			string name = Window.Prompt("Enter a name for the theme");

			LayoutPatch targetLayout = null;
			if (LayoutsComboBox.SelectedIndex > 0)
				targetLayout = layoutPatches[int.Parse(
					((HTMLOptionElement)LayoutsComboBox.Children[LayoutsComboBox.SelectedIndex]).Value)];

			MakeNxTheme(targetPatch.NXThemeName,name,"",targetLayout);
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
				LayoutPatch targetLayout = null;
				if (LayoutsComboBox.SelectedIndex > 0)
					targetLayout = layoutPatches[int.Parse(
						((HTMLOptionElement)LayoutsComboBox.Children[LayoutsComboBox.SelectedIndex]).Value)];
				
				var yaz0 = Theme.Make(CommonSzs, DDSEncoder.LoadDDS(LoadedDDS) , targetPatch,targetLayout);
				if (yaz0 == null) return;
				Uint8Array dwn = new Uint8Array(yaz0);
				string DownloadFname = targetPatch.szsName;
				Script.Write("downloadBlob(dwn,DownloadFname,'application/octet-stream');");
			});
		}

		public readonly static string[] ValidAutoThemeParts = new string[] { "home", "lock", "user", "set", "apps", "news" };
		public static void AutoThemeDeleteAll()
		{
			foreach (var p in ValidAutoThemeParts)
			{
				Window.LocalStorage.RemoveItem(p);
				Window.LocalStorage.RemoveItem(p + "Name");
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

	public class Theme
	{
		public static byte[] Make(SarcData input, DDSLoadResult dds, PatchTemplate targetPatch, LayoutPatch layout)
		{
			if (layout != null)
			{
				var layoutres = SwitchThemesCommon.PatchLayouts(input, layout.Files);
				if (layoutres == BflytFile.PatchResult.Fail)
				{
					Window.Alert("One of the target files for the selected layout patch is missing in the SZS, you are probably using an already patched SZS or the wrong layout");
					return null;
				}
				else if (layoutres == BflytFile.PatchResult.CorruptedFile)
				{
					Window.Alert("A layout in this SZS is missing a pane required for the selected layout patch, you are probably using an already patched SZS or the wrong layout");
					return null;
				}
			}

			if (SwitchThemesCommon.PatchBntx(input, dds, targetPatch) == BflytFile.PatchResult.Fail)
			{
				Window.Alert(
						"Can't build this theme: the szs you opened doesn't contain some information needed to patch the bntx," +
						"without this information it is not possible to rebuild the bntx." +
						"You should use an original or at least working szs");
				return null;
			}
			var res = SwitchThemesCommon.PatchBgLayouts(input, targetPatch);
			if (res == BflytFile.PatchResult.Fail)
			{
				Window.Alert("Couldn't patch this file, it might have been already modified or it's from an unsupported system version.");
				return null;
			}
			else if (res == BflytFile.PatchResult.CorruptedFile)
			{
				Window.Alert("This file has been already patched with another tool and is not compatible, you should get an unmodified layout.");
				return null;
			}
			var sarc = SARC.PackN(input);
			return ManagedYaz0.Compress(sarc.Item2, 1, (int)sarc.Item1);
		}
	}
}