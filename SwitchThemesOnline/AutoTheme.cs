using Bridge.Html5;
using ExtensionMethods;
using SwitchThemes.Common.Bntxx;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using SARCExt;
using SwitchThemes.Common;
using Bridge;

namespace SwitchThemesOnline
{
	public class AutoTheme
	{
		const string Domain = "";

		static HTMLDivElement loader = null;
		static HTMLParagraphElement LoaderText = null;
		public static void OnLoad()
		{
#if DEBUG
			Document.GetElementById<HTMLDivElement>("DebugFlag").Hidden = false;
#endif
			Document.GetElementById<HTMLDivElement>("D_JsWarn").Remove();
			string useragent = Window.Navigator.UserAgent.ToLower();
			if (useragent.Contains("msie") || useragent.Contains("trident"))
			{
				Document.GetElementById<HTMLDivElement>("D_IeWarn").Hidden = false;
			}
			loader = Document.GetElementById<HTMLDivElement>("loaderDiv");
			LoaderText = Document.GetElementById<HTMLParagraphElement>("LoadingText");

			string type = GetUriVar("type");
			if (type != null)
			{
				if (!App.ValidAutoThemeParts.ContainsStr(type))
				{
					Window.Alert("The selected theme type isn't supported, probably you followed an invalid url");
					return;
				}
				if (Window.LocalStorage.GetItem(type + "Name") == null)
				{
					Window.Alert("You didn't setup auto-theme for this theme type. To setup auto-theme go to the home page and click on the Auto-theme tab");
					return;
				}
				string Url = GetUriVar("dds");
				if (Url == null)
				{
					Window.Alert("No url for the DDS has been specified");
					return;
				}
				DoAutoTheme(type, Url);
			}
		}

		static void DoAutoTheme(string type, string url)
		{
			var cardLoad = Document.GetElementById<HTMLDivElement>("CardLoad");

			void endWithError(string error)
			{
				Window.Alert(error);
				cardLoad.InnerHTML = "There was an error generating the theme: <br/>" + error;
				EndLoading();
			}
			
			Document.GetElementById<HTMLDivElement>("CardTutorial").Hidden = true;
			cardLoad.InnerHTML = "Wait while your theme is being generated.... <br/><br/>This theme is for " + Window.LocalStorage.GetItem(type + "Name") as string + "<br/>To change the target version upload another szs for Auto-Theme on the <a href=\"index.html\">Home page</a>";
			cardLoad.Hidden = false;
			StartLoading();
			XMLHttpRequest req = new XMLHttpRequest();
			req.ResponseType = XMLHttpRequestResponseType.ArrayBuffer;
			req.OnReadyStateChange = () =>
			{
				if (req.ReadyState !=  AjaxReadyState.Done) return;
				ArrayBuffer DownloadRes = req.Response as ArrayBuffer;
				if (DownloadRes == null)
				{
					endWithError("DDS download failed, check the url");
					return;
				}
				var arr = new Uint8Array(DownloadRes);
				if (arr.Length == 0)
				{
					endWithError("Error downloading the DDS, most likely the link you provided is not a direct link or the host doesn't support Cross-Origin Resource Sharing");
					return;
				}
				var LoadedDDS = DDSEncoder.LoadDDS(arr.ToArray());
				arr = null;
				DownloadRes = null;
				var CommonSzs = SARC.UnpackRamN(
					ManagedYaz0.Decompress(
						Convert.FromBase64String(
							Window.LocalStorage.GetItem(type) as string)));

				var targetPatch = SwitchThemesCommon.DetectSarc(CommonSzs, DefaultTemplates.templates);
				if (SwitchThemesCommon.PatchBntx(CommonSzs, LoadedDDS, targetPatch) == BflytFile.PatchResult.Fail)
				{
					endWithError(
							"Can't build this theme: the szs you opened doesn't contain some information needed to patch the bntx," +
							"without this information it is not possible to rebuild the bntx." +
							"You should use an original or at least working szs");
					return;
				}

				var res = SwitchThemesCommon.PatchLayouts(CommonSzs, targetPatch);

				if (res == BflytFile.PatchResult.Fail)
				{
					endWithError("Couldn't patch this file, it might have been already modified or it's from an unsupported system version.");
					return;
				}
				else if (res == BflytFile.PatchResult.CorruptedFile)
				{
					endWithError("This file has been already patched with another tool and is not compatible, you should get an unmodified layout.");
					return;
				}
				var sarc = SARC.PackN(CommonSzs);
				byte[] yaz0 = ManagedYaz0.Compress(sarc.Item2, 1, (int)sarc.Item1);
				sarc = null;
				Uint8Array dwn = new Uint8Array(yaz0);
				string DownloadFname = targetPatch.szsName;
				Script.Write("downloadBlob(dwn,DownloadFname,'application/octet-stream');");
				Document.GetElementById<HTMLDivElement>("CardLoad").TextContent = "Your theme has been generated !";
				EndLoading();
			};
			req.Open("GET", url, true);
			req.Send();
		}

		public static void MakeLink()
		{
			string type = Document.GetElementById<HTMLSelectElement>("TargetSelect").Value;
			string url = Document.GetElementById<HTMLInputElement>("DDSUrlInput").Value;
			if (!App.ValidAutoThemeParts.ContainsStr(type))
			{
				Window.Alert("The selected type is invalid");
				return;
			}
			if (url.Length < 5)
			{
				Window.Alert("Enter a valid url");
				return;
			}
			Document.GetElementById<HTMLParagraphElement>("Linkis").Hidden = false;
			Document.GetElementById<HTMLButtonElement>("BtnLinkCopy").Style.Display = Display.Block;
			var str = "https://"+ Domain + "/autotheme.html?type=" + type + "&dds=" + url;
			var link = Document.GetElementById<HTMLLinkElement>("OutLink");
			link.TextContent = str;
			link.Href = str;
		}

		public static void CopyLink()
		{
			HTMLTextAreaElement a = new HTMLTextAreaElement();
			Document.Body.AppendChild(a);
			a.Value = Document.GetElementById<HTMLLinkElement>("OutLink").Href;
			a.Select();
			Document.ExecCommand("copy");
			a.Remove();
		}

		static void StartLoading()
		{
			LoaderText.TextContent = App.loadingFaces[new Random().Next(0, App.loadingFaces.Length)];
			loader.Style.Display = "";
		}

		static void EndLoading()
		{
			loader.Style.Display = "none";
		}

		public static string GetUriVar(string name)
		{
			var query = Window.Location.Search.Substring(1);
			var vars = query.Split('&');
			for (var i = 0; i < vars.Length; i++)
			{
				var pair = vars[i].Split('=');
				if (Window.DecodeURIComponent(pair[0]) == name)
				{
					return Window.DecodeURIComponent(pair[1]);
				}
			}
			return null;
		}

	}
}
