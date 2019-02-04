using SwitchThemes.Common;
using SwitchThemes.Common.Bntxx;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
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
			bool IsMono = Type.GetType("Mono.Runtime") != null;

			if (IsMono)
				AppDomain.CurrentDomain.AssemblyResolve += (sender, Assemblyargs) => {
					String resourceName = "AssemblyLoadingAndReflection." +
					   new AssemblyName(Assemblyargs.Name).Name + ".dll";
					using (var stream = Assembly.GetExecutingAssembly().GetManifestResourceStream(resourceName))
					{
						Byte[] assemblyData = new Byte[stream.Length];
						stream.Read(assemblyData, 0, assemblyData.Length);
						return Assembly.Load(assemblyData);
					}
				};

			bool ArgsHandled = false;
			if (args != null && args.Length != 0)
			{
				if (args[0].ToLower() == "buildnx")
					ArgsHandled = NXThemeFromArgs(args);
				else if (args[0].ToLower() == "help")
				{
					ArgsHandled = true;
					Console.WriteLine(
						"Switch themes Injector V " + SwitchThemesCommon.CoreVer +" by exelix\r\nhttps://github.com/exelix11/SwitchThemeInjector\r\n\r\n" +
						"Usage: SwitchThemes.exe buildNX home \"<your image.png/jpg/dds>\" \"<json layout file, optional>\" \"name=<theme name>\" \"author=<author name>\" \"commonlyt=<custom common.szs layout>\" \"album=<custom album icon.png/dds>\" \"out=<OutputPath>.nxtheme\"\r\n" +
						"instad of home you can use: lock for lockscreen, apps for the all apps screen, set for the settings applet, user for the user page applet and news for the news applet.\r\n"+
						"Only the image and out file are needed.\r\n");
					if (IsMono)
						Console.WriteLine("Note that on linux you MUST use dds images, make sure to use DXT1 encoding for background image and DXT5 for album. Always check with an hex editor, some times ImageMagick uses DXT5 even if DXT1 is specified through command line args");
				}
			}

			if (ArgsHandled)
				return;

			if (IsMono)
			{
				MessageBox.Show("The ui is not supported with mono, use the command line args.\r\nRun \"mono SwitchThemes.exe help\"");
				return;
			}

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
			string album = GetArg("album");

			if (Output == null || Output == "")
				return false;
			
			bool preview = GetArgBool("preview") ?? true;

			if (Name == null || Name.Trim() == "")
			{
				var info = ThemeInputInfo.Ask();
				Name = info.Item1;
				Author = info.Item2;
				preview = info.Item3;
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

			var dds = DDSEncoder.LoadDDS(File.ReadAllBytes(Image));
			if (dds.Format != "DXT1") MessageBox.Show("WARNING: the encoding of the selected DDS is not DXT1, it may crash on the switch");
			if (dds.width != 1280 || dds.height != 720) MessageBox.Show("WARNING: the selected image is not 720p (1280x720), it may crash on the swtich");

			if (album != null && !album.EndsWith(".dds"))
			{
				if (Form1.ImageToDDS(album, Path.GetTempPath()))
					album = Path.Combine(Path.GetTempPath(), Path.GetFileNameWithoutExtension(album) + ".dds");
				else return false;
			}

			var res = SwitchThemesCommon.GenerateNXTheme(
				new ThemeFileManifest()
				{
					Version = 4,
					ThemeName = Name,
					Author = Author,
					Target = Target,
					LayoutInfo = layout == null ? "" : layout.PatchName + " by " + layout.AuthorName,
				},
				File.ReadAllBytes(Image),
				layout?.AsByteArray(),
				new Tuple<string, byte[]>("preview.png", preview ? Form1.GenerateDDSPreview(Image) : null),
				new Tuple<string, byte[]>("album.dds", album != null ? File.ReadAllBytes(album) : null));

			File.WriteAllBytes(Output, res);

			return true;
		}
	}
}
