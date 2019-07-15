using SARCExt;
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
				else if (args[0].ToLower() == "szs")
					ArgsHandled = SZSFromArgs(args);
				else if (args[0].ToLower() == "help")
				{
					ArgsHandled = true;
					Console.WriteLine(
						"Switch themes Injector V " + SwitchThemesCommon.CoreVer +" by exelix\r\nhttps://github.com/exelix11/SwitchThemeInjector\r\n\r\n" +
						"Usage: SwitchThemes.exe buildNX home \"<your image.png/jpg/dds>\" \"<json layout file, optional>\" \"name=<theme name>\" \"author=<author name>\" \"commonlyt=<custom common.szs layout>\" \"out=<OutputPath>.nxtheme\"\r\n" +
						"instead of home you can use: lock for lockscreen, apps for the all apps screen, set for the settings applet, user for the user page applet and news for the news applet.\r\n"+
						"Only the image and out file are needed.\r\n" +
						"To patch SZS files: SwitchThemes.exe szs \"<input file>\" \"<your image.png/jpg/dds>\" \"<json layout file, optional>\" \"out=<OutputPath>.szs\"\r\n");
					Console.WriteLine("The following applet icons are supported as well: " + string.Join(", ", AppletButtonPatch.Patches.Select(x => x.NxThemeName).ToArray()));
					if (IsMono)
						Console.WriteLine("Note that on linux you MUST use dds images, make sure to use DXT1 encoding for background image and DXT5 for album. Always check with an hex editor, some times ImageMagick uses DXT5 even if DXT1 is specified through command line args");
				}
			}

			if (ArgsHandled)
				return;

			if (IsMono)
			{
				Console.WriteLine("The ui is not supported with mono, use the command line args.\r\nRun \"mono SwitchThemes.exe help\"");
				return;
			}

			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			Application.Run(new Form1());
		}

		static bool SZSFromArgs(string[] args)
		{
			string GetArg(string start)
			{
				var a = args.Where(x => x.StartsWith(start + "=")).FirstOrDefault();
				if (a == null) return null;
				else return a.Split('=')[1];
			}

			if (args.Length < 2)
				return false;

			string Target = args[1];
			var	CommonSzs = SARCExt.SARC.UnpackRamN(ManagedYaz0.Decompress(File.ReadAllBytes(Target)));
			var targetPatch = SzsPatcher.DetectSarc(CommonSzs, DefaultTemplates.templates);

			if (targetPatch == null)
			{
				Console.WriteLine("Unknown szs file");
				return false;
			}

			string Image = args.Where(x => x.EndsWith(".dds") || x.EndsWith(".jpg") || x.EndsWith(".png") || x.EndsWith("jpeg")).FirstOrDefault();
			if (Image == null || !File.Exists(Image))
			{
				Console.WriteLine("No image file !");
				return false;
			}
			string Layout = args.Where(x => x.EndsWith(".json")).FirstOrDefault();
			
			string Output = GetArg("out");

			if (Output == null || Output == "")
				return false;

			if (!Image.EndsWith(".dds"))
			{
				if (Form1.ImageToDDS(Image, Path.GetTempPath()))
					Image = Path.Combine(Path.GetTempPath(), Path.GetFileNameWithoutExtension(Image) + ".dds");
				else return false;
			}

			Dictionary<string, string> AppletIcons = new Dictionary<string, string>();
			foreach (var a in AppletButtonPatch.Patches)
			{
				string path = GetArg(a.NxThemeName);
				if (!path.EndsWith(".dds") && !Form1.IcontoDDS(ref path))
					path = null;
				AppletIcons.Add(a.NxThemeName, path);
			}			

			try
			{				
				var res = BflytFile.PatchResult.OK;
				var Patcher = new SzsPatcher(CommonSzs, DefaultTemplates.templates);

				if (Image != null)
				{
					res = Patcher.PatchMainBG(File.ReadAllBytes(Image));
					if (res == BflytFile.PatchResult.Fail)
					{
						Console.WriteLine("Couldn't patch this file, it might have been already modified or it's from an unsupported system version.");
						return false;
					}
					else if (res == BflytFile.PatchResult.CorruptedFile)
					{
						Console.WriteLine("This file has been already patched with another tool and is not compatible, you should get an unmodified layout.");
						return false;
					}
				}

				if (targetPatch.szsName == "ResidentMenu.szs")
				{
					foreach (var a in AppletButtonPatch.Patches)
					{
						if (AppletIcons[a.NxThemeName] != null)
							Patcher.PatchBntxTexture(File.ReadAllBytes(AppletIcons[a.NxThemeName]), a.BntxName, a.NewColorFlags);
					}
				}

				if (Layout != null)
				{
					Patcher.EnableAnimations = true;
					var l = LayoutPatch.LoadTemplate(File.ReadAllText(Layout));
					var layoutres = Patcher.PatchLayouts(l, targetPatch.NXThemeName, targetPatch.NXThemeName == "home");
					if (layoutres == BflytFile.PatchResult.Fail)
					{
						Console.WriteLine("One of the target files for the selected layout patch is missing in the SZS, you are probably using an already patched SZS");
						return false;
					}
					else if (layoutres == BflytFile.PatchResult.CorruptedFile)
					{
						Console.WriteLine("A layout in this SZS is missing a pane required for the selected layout patch, you are probably using an already patched SZS");
						return false;
					}
					layoutres = Patcher.PatchAnimations(l.Anims);
				}

				CommonSzs = Patcher.GetFinalSarc();
				var sarc = SARC.PackN(CommonSzs);

				File.WriteAllBytes(Output, ManagedYaz0.Compress(sarc.Item2, 3, (int)sarc.Item1));
				GC.Collect();

				if (res == BflytFile.PatchResult.AlreadyPatched)
					Console.WriteLine("Done, This file has already been patched before.\r\nIf you have issues try with an unmodified file");
				else
					Console.WriteLine("Done");
			}
			catch (Exception ex)
			{
				Console.WriteLine("ERROR: " + ex.Message);
			}

			return true;
		}

		static bool NXThemeFromArgs(string[] args)
		{
			if (args.Length < 4)
				return false;
			string Target = args[1];
			if (!Form1.HomeMenuParts.Values.Contains(Target))
				return false;

			string Image = args.Where(x => x.EndsWith(".dds") || x.EndsWith(".jpg") || x.EndsWith(".png") || x.EndsWith("jpeg")).FirstOrDefault();
			if (Image != null && !File.Exists(Image))
			{
				Console.WriteLine("No image file !");
				return false;
			}

			string Layout = args.Where(x => x.EndsWith(".json")).FirstOrDefault();
			if (Image == null && Layout == null)
			{
				Console.WriteLine("You need at least an image or a layout to make a theme");
				return false;
			}

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
			{
				Console.WriteLine("Missing out= arg");
				return false;
			}

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

			if (Image != null && !Image.EndsWith(".dds"))
			{
				if (Form1.ImageToDDS(Image, Path.GetTempPath()))
					Image = Path.Combine(Path.GetTempPath(), Path.GetFileNameWithoutExtension(Image) + ".dds");
				else return false;
			}

			Dictionary<string, string> AppletIcons = new Dictionary<string, string>();
			foreach (var a in AppletButtonPatch.Patches)
			{
				string path = GetArg(a.NxThemeName);
				if (!path.EndsWith(".dds") && !Form1.IcontoDDS(ref path))
					path = null;
				AppletIcons.Add(a.NxThemeName, path);
			}

			try
			{
				var builder = new NXThemeBuilder(Target, Name, Author);

				if (layout != null)
					builder.AddMainLayout(layout);
				if (Image != null)
					builder.AddMainBg(File.ReadAllBytes(Image));
				if (ExtraCommon != null)
					builder.AddFile("common.json", File.ReadAllBytes(ExtraCommon));

				foreach (var i in AppletIcons)
					if (i.Value != null)
						builder.AddAppletIcon(i.Key, File.ReadAllBytes(i.Value));

				File.WriteAllBytes(Output, builder.GetNxtheme());
			}
			catch (Exception ex)
			{
				Console.WriteLine("ERROR: " + ex.Message);
				return false;
			}

			return true;
		}
	}
}
