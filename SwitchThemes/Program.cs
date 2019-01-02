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

			string Image = args.Where(x => x.EndsWith(".dds") || x.EndsWith(".jpg") || x.EndsWith(".png") || x.EndsWith("jpeg")).First();
			if (Image == null || !File.Exists(Image))
			{
				Console.WriteLine("No image file !");
				return false;
			}
			string Layout = args.Where(x => x.EndsWith(".json")).First();

			string GetArg(string start)
			{
				var a = args.Where(x => x.StartsWith(start + "=")).First();
				if (a == null) return null;
				else return a.Split('=')[1];
			}
			
			string Name = GetArg("name");
			string Author = GetArg("author");
			string Output = GetArg("out");
			if (Output == null || Output == "")
				return false;

			if (Name == null || Name.Trim() == "")
			{
				var info = ThemeInputInfo.Ask();
				Name = info.Item1;
				Author = info.Item2;
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
					Version = 2,
					ThemeName = Name,
					Author = Author,
					Target = Target,
					LayoutInfo = layout == null ? "" : layout.PatchName + " by " + layout.AuthorName,
					UseCommon5X = true
				},
				File.ReadAllBytes(Image),
				layout?.AsJson(),
				Form1.GenerateDDSPreview(Image));

			File.WriteAllBytes(Output, res);

			return true;
		}
	}
}
