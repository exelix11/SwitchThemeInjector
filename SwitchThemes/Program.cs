using SARCExt;
using SwitchThemes.Common;
using SwitchThemes.Common.Bntxx;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SwitchThemes
{
	static class Program
	{
		[DllImport("kernel32.dll")]
		static extern bool AttachConsole(int dwProcessId);
		private const int ATTACH_PARENT_PROCESS = -1;

		/// <summary>
		/// Punto di ingresso principale dell'applicazione.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
			//Used to generate json patches for the installer
			//Clipboard.SetText(TextureReplacement.GenerateJsonPatchesForInstaller());

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
				if (!IsMono) AttachConsole(ATTACH_PARENT_PROCESS);

				if (args[0].ToLower() == "buildnx")
					NXThemeFromArgs(args);
				else if (args[0].ToLower() == "szs")
					SZSFromArgs(args);
				else if (args[0].ToLower() == "install")
					RemoteInstallFromArgs(args);
				else if (args[0].ToLower() == "extract")
					ExtractNxtheme(args);
				else if (args[0].ToLower() == "diff")
					DiffSzs(args);
				else if (args[0].ToLower() == "help")
				{
					Console.WriteLine(
						"Switch themes Injector V " + Info.CoreVer + " by exelix\r\nhttps://github.com/exelix11/SwitchThemeInjector\r\n\r\n" +
						"Command line usage:\r\n" +
						"Build an nxtheme file : SwitchThemes.exe buildNX home \"<your image.jpg/dds>\" \"<json layout file, optional>\" \"name=<theme name>\" \"author=<author name>\" \"commonlyt=<custom common.szs layout>\" \"out=<OutputPath>.nxtheme\"\r\n" +
						" instead of home you can use: lock for lockscreen, apps for the all apps screen, set for the settings applet, user for the user page applet and news for the news applet.\r\n" +
						" Only the image and out file are needed.\r\n" +
						"Patch an SZS: SwitchThemes.exe szs \"<input file>\" \"<your image.dds, optional>\" \"<json layout file, optional>\" \"out=<OutputPath>.szs\"\r\n" +
						"Extract an nxtheme: Switchthemes.exe extract \"<input file>\" \"<target oath>\" \r\n" +
						"Remote install to NXTheme installer: SwitchThemes.exe install 192.168.X.Y \"<your nxtheme/szs file>\"\r\n" +
						"Diff szs files: SwitchThemes.exe diff <original szs file> <modified szs file> <output json path>\r\n");
					Console.WriteLine("The following applet icons are supported for home menu: " + string.Join(", ", TextureReplacement.ResidentMenu.Select(x => $"{x.NxThemeName} ({x.W}x{x.H})").ToArray()));
					Console.WriteLine("The following applet icons are supported for the lock screen: " + string.Join(", ", TextureReplacement.Entrance.Select(x => $"{x.NxThemeName} ({x.W}x{x.H})").ToArray()));
					Console.WriteLine("Applet icons only support png and dds images");
				}

				ArgsHandled = true;
			}

			if (ArgsHandled)
				return;

			if (IsMono)
			{
				Console.Error.WriteLine("The ui is not supported with mono, use the command line args.\r\nRun \"mono SwitchThemes.exe help\"");
				return;
			}

			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			Application.Run(new Form1());
		}

		static bool RemoteInstallFromArgs(string[] args)
		{
			if (args.Length != 3)
			{
				Console.Error.WriteLine("Error: Wrong number of arguments.");
				return false;
			}

			string Ip = args[1];
			byte[] Theme = File.ReadAllBytes(args[2]);

			var res = RemoteInstallForm.DoRemoteInstall(Ip, Theme);
			if (res == null) {
				Console.WriteLine("Done!");
			} else {
				Console.Error.WriteLine(res);
			}

			return true;
		}

		static bool DiffSzs(string[] args)
		{
			if (args.Length != 4)
			{
				Console.Error.WriteLine("Error: Wrong number of arguments.");
				return false;
			}

			var original = args[1];
			var edited = args[2];
			var outName = args[3];

			var options = new LayoutDiff.DiffOptions {
				HideOnlineButton = args.Any(x => x == "--hide-online")
			};

			try
			{
				var res = LayoutDiff.Diff(
					SARC.Unpack(ManagedYaz0.Decompress(File.ReadAllBytes(original))),
					SARC.Unpack(ManagedYaz0.Decompress(File.ReadAllBytes(edited))),
					options
				);

				File.WriteAllBytes(outName, res.Item1.AsByteArray());

				Console.WriteLine(res.Item2);
			}
			catch (Exception ex)
			{
				Console.Error.WriteLine($"There was an error:\r\n{ex}");
			}

			return true;
		}

		static bool ExtractNxtheme(string[] args)
		{
			if (args.Length != 3)
			{
				Console.Error.WriteLine("Error: Wrong number of arguments.");
				return false;
			}

			try
			{
				Form1.DoExtractNxTheme(args[1], args[2]);
				Console.WriteLine("Done!");
			}
			catch (Exception ex)
			{
				Console.Error.WriteLine($"There was an error:\r\n{ex}");
			}

			return true;
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
			var	TargetSzs = SARCExt.SARC.Unpack(ManagedYaz0.Decompress(File.ReadAllBytes(Target)));
			var targetPatch = DefaultTemplates.GetFor(TargetSzs);

			if (targetPatch == null)
			{
				Console.Error.WriteLine("Unknown SZS file.");
				return false;
			}

			string Image = args.Where(x => x.ToLower().EndsWith(".dds")).FirstOrDefault();
			if (Image != null && !File.Exists(Image))
			{
				Console.Error.WriteLine("DDS image not found!\r\nNote that only DDS files are supported for szs themes.");
				return false;
			}

			string Layout = args.Where(x => x.EndsWith(".json")).FirstOrDefault();
			if (Layout != null && !File.Exists(Layout)) {
				Console.Error.WriteLine("JSON layout not found!");
				return false;
			}

			string Output = GetArg("out");
			if (Output == null || Output == "")
			{
				Console.Error.WriteLine("No output path supplied! Example: 'out=file.szs'");
				return false;
			}

			if (Image == null && Layout == null)
			{
				Console.Error.WriteLine("Nothing to do! An image (DDS), layout (JSON), or both should be supplied.");
				return false;
			}

			try {				
				var res = true;
				var Patcher = new SzsPatcher(TargetSzs);

				if (Image != null)
				{
					{
						var dds = Common.Images.Util.ParseDds(File.ReadAllBytes(Image));
						if (dds.Encoding != "DXT1") Console.WriteLine("WARNING: the encoding of the selected DDS is not DXT1, it may crash on the switch");
						if (dds.Size.Width != 1280 || dds.Size.Height != 720) Console.WriteLine("WARNING: the selected image is not 720p (1280x720), it may crash on the switch");
					}

					res = Patcher.PatchMainBG(File.ReadAllBytes(Image));
					if (!res)
					{
						Console.Error.WriteLine("Couldn't patch this file, it might have been already modified or it's from an unsupported system version.");
						return false;
					}
				}

				void ProcessAppletIcons(List<TextureReplacement> l)
				{
					foreach (var a in l)
					{
						string path = GetArg(a.NxThemeName);
						if (path != null && !path.EndsWith(".dds"))
						{
							Console.Error.WriteLine($"{path} is not supported, only dds files can be used for szs themes");
							path = null;
						}
						if (path != null)
							if (!Patcher.PatchAppletIcon(File.ReadAllBytes(path), a.NxThemeName))
								Console.Error.WriteLine($"Applet icon patch for {a.NxThemeName} failed");
					}
				}

				if (TextureReplacement.NxNameToList.ContainsKey(targetPatch.NXThemeName))
					ProcessAppletIcons(TextureReplacement.NxNameToList[targetPatch.NXThemeName]);

				if (Layout != null)
				{
					var l = LayoutPatch.Load(File.ReadAllText(Layout));
					var layoutres = Patcher.PatchLayouts(l);
					if (!layoutres)
					{
						Console.Error.WriteLine("One or more of the target files for the selected layout patch is missing in the SZS. Either this layout it not meant for this menu or you are using an already patched SZS.");
						return false;
					}
				}

				TargetSzs = Patcher.GetFinalSarc();
				var sarc = SARC.Pack(TargetSzs);

				File.WriteAllBytes(Output, ManagedYaz0.Compress(sarc.Item2, 3, (int)sarc.Item1));
				GC.Collect();

				if (Image != null && Patcher.PatchTemplate.RequiresCodePatch)
					Console.WriteLine("The file has been patched successfully but due to memory limitations this szs requires an extra code patch to be applied to the home menu, if you use NXThemesInstaller to install this it will be done automatically, otherwise you need to manually copy the patches from https://github.com/exelix11/SwitchThemeInjector/tree/master/SwitchThemesNX/romfs to the exefs patches directory of your cfw");
				else
					Console.WriteLine("Done!");
			}
			catch (Exception ex)
			{
				Console.Error.WriteLine("Error: " + ex.Message);
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
				Console.Error.WriteLine("No image file!");
				return false;
			}

			string Layout = args.Where(x => x.EndsWith(".json")).FirstOrDefault();
			if (Image == null && Layout == null)
			{
				Console.Error.WriteLine("You need at least an image or a layout to make a theme.");
				return false;
			}

			string GetArg(string start)
			{
				var a = args.Where(x => x.StartsWith(start + "=")).FirstOrDefault();
				if (a == null) return null;
				else return a.Split('=')[1];
			}
			
			string Name = GetArg("name");
			string Author = GetArg("author");
			string Output = GetArg("out");
			string ExtraCommon = GetArg("commonlyt");
			string album = GetArg("album");

			if (Output == null || Output == "")
			{
				Console.Error.WriteLine("Missing out= arg");
				return false;
			}

			if (Name == null || Name.Trim() == "")
			{
				(Name, Author) = ThemeInputInfo.Ask();
				if (Name == null) return true;	
			}

			LayoutPatch layout = null;
			if (Layout != null && File.Exists(Layout))
				layout = LayoutPatch.Load(File.ReadAllText(Layout));

			Dictionary<string, string> AppletIcons = new Dictionary<string, string>();
			void PopulateAppletIcons(List<TextureReplacement> l)
			{
				foreach (var a in l)
				{
					string path = GetArg(a.NxThemeName);
					AppletIcons.Add(a.NxThemeName, path);
				}
			}

			if (TextureReplacement.NxNameToList.ContainsKey(Target))
				PopulateAppletIcons(TextureReplacement.NxNameToList[Target]);

			try
			{
				var builder = new NXThemeBuilder(Target, Name, Author);

				if (layout != null)
					builder.AddMainLayout(layout);
				if (Image != null)
					builder.AddMainBg(File.ReadAllBytes(Image));
				if (ExtraCommon != null)
					builder.AddCommonLayout(File.ReadAllText(ExtraCommon));

				foreach (var i in AppletIcons)
					if (i.Value != null)
						builder.AddAppletIcon(i.Key, File.ReadAllBytes(i.Value));

				File.WriteAllBytes(Output, builder.GetNxtheme());
				Console.WriteLine("Done!");
			}
			catch (Exception ex)
			{
				Console.Error.WriteLine("Error: " + ex.Message);
				return false;
			}

			return true;
		}
	}
}
