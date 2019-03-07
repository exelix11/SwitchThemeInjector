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
		const string Domain = "exelix11.github.io/SwitchThemeInjector";

		static HTMLDivElement loader = null;
		static HTMLParagraphElement LoaderText = null;
		static HTMLDivElement cardLoad;
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
				string Url = GetUriVar("dds");
				if (Url == null)
				{
					Window.Alert("No url for the DDS has been specified");
					return;
				}
				string layout = GetUriVar("layout");
				DoAutoTheme(type, Url, layout);
			}

			//todo : dynamically add options for the layout
		}

		static void endWithError(string error)
		{			
			Window.Alert(error);
			cardLoad.InnerHTML = "There was an error generating the theme: <br/>" + error;
			EndLoading();
		}

		const string DwnErr = "Error downloading the {0}, most likely the link you provided is not a direct link or the host doesn't support Cross-Origin Resource Sharing";
		static void HttpRequest(string url, Action<Uint8Array> CallBack, string partName)
		{
			XMLHttpRequest req = new XMLHttpRequest();
			req.ResponseType = XMLHttpRequestResponseType.ArrayBuffer;
			req.OnReadyStateChange = () =>
			{
				if (req.ReadyState != AjaxReadyState.Done) return;
				ArrayBuffer DownloadRes = req.Response as ArrayBuffer;
				if (DownloadRes == null)
				{
					endWithError(partName + " download failed, check the url");
					return;
				}
				var arr = new Uint8Array(DownloadRes);
				DownloadRes = null;
				if (arr.Length == 0)
				{
					endWithError(string.Format(DwnErr, partName));
					return;
				}
				CallBack(arr);
			};
			req.Open("GET", url, true);
			req.Send();
		}

		public static void HttpRequest(string url, Action<string> CallBack, string partName)
		{
			XMLHttpRequest req = new XMLHttpRequest();
			req.ResponseType = XMLHttpRequestResponseType.String;
			req.OnReadyStateChange = () =>
			{
				if (req.ReadyState != AjaxReadyState.Done) return;
				string DownloadRes = req.Response as string;
				if (DownloadRes == null || DownloadRes.Length == 0)
				{
					endWithError(string.Format(DwnErr, partName));
					return;
				}
				CallBack(DownloadRes);
			};
			req.Open("GET", url, true);
			req.Send();
		}

		static void DoAutoTheme(string type, string url,string layout)
		{
			cardLoad = Document.GetElementById<HTMLDivElement>("CardLoad");
			Document.GetElementById<HTMLDivElement>("CardTutorial").Hidden = true;
			string themeTarget = "<br/><br/>This theme is an nxtheme, download NXThemes Installer to install it on your switch.";
			cardLoad.InnerHTML = "Wait while your theme is being generated.... " + themeTarget;
			cardLoad.Hidden = false;
			StartLoading();

			byte[] DDS;
			LayoutPatch targetLayout = null;

			void BuildTheme()
			{
				var urlSplit = url.Split("/");
				var meta = new ThemeFileManifest()
				{
					Version = 1,
					Author = "Auto-Theme",
					LayoutInfo = targetLayout != null ? targetLayout.ToString() : "",
					ThemeName = urlSplit[urlSplit.Length - 1],
					Target = type
				};

				var res = SwitchThemesCommon.GenerateNXTheme(meta, DDS, targetLayout == null ? null : System.Text.Encoding.UTF8.GetBytes(targetLayout.AsJson()));
				if (res == null)
				{
					endWithError("GenerateNXTheme() failed :(");
					return;
				}
				Uint8Array dwn = new Uint8Array(res);
				string DownloadFname = urlSplit[urlSplit.Length - 1] + ".nxtheme";
				Script.Write("downloadBlob(dwn,DownloadFname,'application/octet-stream');");
				Document.GetElementById<HTMLDivElement>("CardLoad").InnerHTML = "Your theme has been generated !" + themeTarget;
				EndLoading();
			}

			void DDSDownloaded(Uint8Array arr)
			{
				DDS = arr.ToArray();				

				if (layout == null)
					BuildTheme();
				else
					HttpRequest(layout, LayoutDownloaded, "Layout");
			}

			void LayoutDownloaded(string req)
			{
				targetLayout = LayoutPatch.LoadTemplate(req);
				BuildTheme();
			}

			HttpRequest(url, DDSDownloaded, "DDS");
		}

		public static void MakeLink()
		{
			string type = Document.GetElementById<HTMLSelectElement>("TargetSelect").Value;
			string url = Document.GetElementById<HTMLInputElement>("DDSUrlInput").Value;
			string layout = Document.GetElementById<HTMLInputElement>("LayoutUrlInput").Value.Trim();
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
			var str =
				"https://" + Domain + "/autotheme.html?type=" + type +
				"&dds=" + Window.EncodeURIComponent(url);
			if (layout != "")
				str += "&layout=" + Window.EncodeURIComponent(layout);
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

		static void EndLoading() =>	loader.Style.Display = "none";		

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
