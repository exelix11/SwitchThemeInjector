using SwitchThemes.Common.Bntxx;
using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace SwitchThemes.Common
{
    static class SwitchThemesCommon
    {
		public const string CoreVer = "3.8";
		const string LoadFileText =
			"For SZS these are the patches available in this version: (This doesn't affect nxthemes)" +
			"{0} \r\n";

		public static byte[] GenerateNXTheme(ThemeFileManifest info, byte[] image, byte[] layout = null, params Tuple<string,byte[]>[] ExtraFiles)
		{
			{
				var img = DDSEncoder.LoadDDS(image);
				if (img.width != 1280 || img.height != 720 || img.Format != "DXT1")
					throw new Exception("The background image must be 1280x720 and (if you're using a DDS) DXT1 encoded ");
			}

			{
				var album_img = ExtraFiles.Where(x => x.Item1 == "album.dds").FirstOrDefault();
				if (album_img != null && album_img.Item2 != null)
				{
					var img = DDSEncoder.LoadDDS(album_img.Item2);
					if (img.width != 64 || img.height != 56)
						throw new Exception("The custom album image must be 64x56");
				}
			}

			Dictionary<string, byte[]> Files = new Dictionary<string, byte[]>();
			Files.Add("info.json", Encoding.UTF8.GetBytes(info.Serialize()));
			Files.Add("image.dds", image);
			if (layout != null)
				Files.Add("layout.json", layout);

			foreach (var f in ExtraFiles)
				if (f != null && f.Item1 != null && f.Item2 != null)
					Files.Add(f.Item1, f.Item2);

			var sarc = SARCExt.SARC.PackN(new SARCExt.SarcData() {  endianness = ByteOrder.LittleEndian, Files = Files, HashOnly = false} );
			return ManagedYaz0.Compress(sarc.Item2, 1, (int)sarc.Item1);
		}

		public static string GeneratePatchListString(IEnumerable<PatchTemplate> Templates)
		{
			var sortedTemplates = Templates.OrderBy(x => x.FirmName).Reverse();

			string curSection = "";
			string FileList = "";
			foreach (var p in sortedTemplates)
			{
				if (curSection != p.FirmName)
				{
					curSection = p.FirmName;
					FileList += $"\r\nFor {curSection}: \r\n";
				}
				FileList += $"  - {p.TemplateName} : the file is called {p.szsName} from title {p.TitleId}\r\n";
			}
			return string.Format(LoadFileText, FileList);
		}

		public static BflytFile.PatchResult PatchLayouts(SARCExt.SarcData sarc, LayoutFilePatch[] Files)
		{
			foreach (var p in Files)
			{
				if (!sarc.Files.ContainsKey(p.FileName))
					return BflytFile.PatchResult.Fail;
				var target = new BflytFile(sarc.Files[p.FileName]);
				var res = target.ApplyLayoutPatch(p.Patches);
				if (res != BflytFile.PatchResult.OK)
					return res;
				sarc.Files[p.FileName] = target.SaveFile();
			}
			return BflytFile.PatchResult.OK;
		}

		public static BflytFile.PatchResult PatchBgLayouts(SARCExt.SarcData sarc, PatchTemplate template)
		{
			BflytFile BflytFromSzs(string name) => new BflytFile(sarc.Files[name]);
			var layouts = sarc.Files.Keys.Where(x => x.StartsWith("blyt/") && x.EndsWith(".bflyt") && x != template.MainLayoutName).ToArray();
			foreach (var f in layouts)
			{
				BflytFile curTarget = BflytFromSzs(f);
				if (curTarget.PatchTextureName(template.MaintextureName, template.SecondaryTexReplace))
					sarc.Files[f] = curTarget.SaveFile();
			}
			BflytFile MainFile = BflytFromSzs(template.MainLayoutName);
			var res = MainFile.PatchBgLayout(template);
			sarc.Files[template.MainLayoutName] = MainFile.SaveFile();
			return res;
		}

		public static BflytFile.PatchResult PatchBntx(SARCExt.SarcData sarc, byte[] DDS, PatchTemplate targetPatch)
		{
			QuickBntx q = new QuickBntx(new BinaryDataReader(new MemoryStream(sarc.Files[@"timg/__Combined.bntx"])));
			if (q.Rlt.Length != 0x80)
			{
				return BflytFile.PatchResult.Fail;
			}
			q.ReplaceTex(targetPatch.MaintextureName, DDS);
			DDS = null;
			sarc.Files[@"timg/__Combined.bntx"] = null;
			sarc.Files[@"timg/__Combined.bntx"] = q.Write();
			return BflytFile.PatchResult.OK;
		}

		public static BflytFile.PatchResult PatchBntxTexture(SARCExt.SarcData sarc, byte[] DDS, string texName, uint TexFlag = 0xFFFFFFFF)
		{
			QuickBntx q = new QuickBntx(new BinaryDataReader(new MemoryStream(sarc.Files[@"timg/__Combined.bntx"])));
			if (q.Rlt.Length != 0x80)
			{
				return BflytFile.PatchResult.Fail;
			}
			q.ReplaceTex(texName, DDS);
			if (TexFlag != 0xFFFFFFFF)
				q.Textures.Where(x => x.Name == texName).First().ChannelTypes = (int)TexFlag;
			sarc.Files[@"timg/__Combined.bntx"] = null;
			sarc.Files[@"timg/__Combined.bntx"] = q.Write();
			return BflytFile.PatchResult.OK;
		}

		public static BflytFile.PatchResult PatchBntx(SARCExt.SarcData sarc, DDSEncoder.DDSLoadResult DDS, PatchTemplate targetPatch)
		{
			QuickBntx q = new QuickBntx(new BinaryDataReader(new MemoryStream(sarc.Files[@"timg/__Combined.bntx"])));
			if (q.Rlt.Length != 0x80)
			{
				return BflytFile.PatchResult.Fail;
			}
			q.ReplaceTex(targetPatch.MaintextureName, DDS);
			DDS = null;
			sarc.Files[@"timg/__Combined.bntx"] = null;
			sarc.Files[@"timg/__Combined.bntx"] = q.Write();
			return BflytFile.PatchResult.OK;
		}

		public static PatchTemplate DetectSarc(SARCExt.SarcData sarc, IEnumerable<PatchTemplate> Templates)
		{
			bool SzsHasKey(string key) => sarc.Files.ContainsKey(key);

			if (!SzsHasKey(@"timg/__Combined.bntx"))
				return null;

			foreach (var p in Templates)
			{
				if (!SzsHasKey(p.MainLayoutName))
					continue;
				bool isTarget = true;
				foreach (string s in p.FnameIdentifier)
				{
					if (!SzsHasKey(s))
					{
						isTarget = false;
						break;
					}
				}
				if (!isTarget) continue;
				foreach (string s in p.FnameNotIdentifier)
				{
					if (SzsHasKey(s))
					{
						isTarget = false;
						break;
					}
				}
				if (!isTarget) continue;
				return p;
			}
			return null;
		}

		public static Dictionary<string, string> PartToFileName = new Dictionary<string, string>() {
			{"home","ResidentMenu.szs"},
			{"lock","Entrance.szs"},
			{"user","MyPage.szs"},
			{"apps","Flaunch.szs"},
			{"set","Set.szs"},
			{"news","Notification.szs"},
		};

	}
}
