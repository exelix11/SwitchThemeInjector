using SwitchThemes.Common;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SwitchThemes
{
	static class Program
	{
		/// <summary>
		/// Punto di ingresso principale dell'applicazione.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
			bool ArgsHandled = false;
			if (args != null && args.Length != 0)
			{
				if (args[0].ToLower() == "buildnx")
					ArgsHandled = NXThemeFromArgs(args);
				else
					ArgsHandled = false;
			}

			if (ArgsHandled)
				return;

			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			Application.Run(new Form1());
		}

		static bool NXThemeFromArgs(string[] args)
		{
			if (args.Length < 4)
				return false;
			string Target = args[1];
			if (!Form1.HomeMenuParts.Values.Contains(Target))
				return false;

			string Image = args.Where(x => x.EndsWith(".dds") || x.EndsWith(".jpg") || x.EndsWith(".png") || x.EndsWith("jpeg")).FirstOrDefault();
			if (Image == null || !File.Exists(Image))
			{
				Console.WriteLine("No image file !");
				return false;
			}
			string Layout = args.Where(x => x.EndsWith(".json")).FirstOrDefault();

			string GetArg(string start)
			{
				var a = args.Where(x => x.StartsWith(start + "=")).FirstOrDefault();
				if (a == null) return null;
				else return a.Split('=')[1];
			}

			bool? GetArgBool(string start)
			{
				var a = GetArg(start);
				if (a == null) return null;
				else return bool.Parse(a);
			}
			
			string Name = GetArg("name");
			string Author = GetArg("author");
			string Output = GetArg("out");
			string ExtraCommon = GetArg("commonlyt");
			if (Output == null || Output == "")
				return false;
			
			bool Common5x = GetArgBool("common5x") ?? true;
			bool preview = GetArgBool("preview") ?? true;

			if (Name == null || Name.Trim() == "")
			{
				var info = ThemeInputInfo.Ask();
				Name = info.Item1;
				Author = info.Item2;
				Common5x = info.Item3;
				preview = info.Item4;
			}

			LayoutPatch layout = null;
			if (Layout != null && File.Exists(Layout))
				layout = LayoutPatch.LoadTemplate(File.ReadAllText(Layout));

			if (!Image.EndsWith(".dds"))
			{
				if (Form1.ImageToDDS(Image, Path.GetTempPath()))
					Image = Path.Combine(Path.GetTempPath(), Path.GetFileNameWithoutExtension(Image) + ".dds");
				else return false;
			}

			var res = SwitchThemesCommon.GenerateNXTheme(
				new ThemeFileManifest()
				{
					Version = 3,
					ThemeName = Name,
					Author = Author,
					Target = Target,
					LayoutInfo = layout == null ? "" : layout.PatchName + " by " + layout.AuthorName,
					UseCommon5X = Common5x
				},
				File.ReadAllBytes(Image),
				layout?.AsByteArray(),
				new Tuple<string, byte[]>("preview.png", preview ? Form1.GenerateDDSPreview(Image) : null));

			File.WriteAllBytes(Output, res);

			return true;
		}
	}
}
