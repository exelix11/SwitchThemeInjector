using SwitchThemes.Common.Bntxx;
using SwitchThemes.Common.Serializers;
using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using SARCExt;
using ExtensionMethods;

namespace SwitchThemes.Common
{
	public static class SwitchThemesCommon
	{
		public const string CoreVer = "4.2";
		public const int NxThemeFormatVersion = 8;

		const string LoadFileText =
			"For SZS these are the patches available in this version: (This doesn't affect nxthemes)" +
			"{0} \r\n";

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

		public static Dictionary<string, string> PartToFileName = new Dictionary<string, string>() {
			{"home","ResidentMenu.szs"},
			{"lock","Entrance.szs"},
			{"user","MyPage.szs"},
			{"apps","Flaunch.szs"},
			{"set","Set.szs"},
			{"news","Notification.szs"},
			//{ "opt","Option.szs" },
			{ "psl","Psl.szs" },
		};

	}

	public class NXThemeBuilder
	{
		private Dictionary<string, byte[]> files = new Dictionary<string, byte[]>();
		ThemeFileManifest info;

		public NXThemeBuilder(string target, string name, string author)
		{
			info = new ThemeFileManifest()
			{
				Version = SwitchThemesCommon.NxThemeFormatVersion,
				ThemeName = name,
				Author = author,
				Target = target,
			};
		}

		public byte[] GetNxtheme()
		{
			if (!files.ContainsKey("image.dds") && !files.ContainsKey("image.png") && !files.ContainsKey("layout.json"))
				throw new Exception("An nxtheme must contain at least a custom background image or layout");

			if (!files.ContainsKey("info.json"))
				AddFile("info.json", Encoding.UTF8.GetBytes(info.Serialize()));

			var sarc = SARCExt.SARC.PackN(new SARCExt.SarcData() { endianness = ByteOrder.LittleEndian, Files = files, HashOnly = false });
			return ManagedYaz0.Compress(sarc.Item2, 1, (int)sarc.Item1);
		}

		public void AddFile(string name, byte[] data)
		{
			if (info.Target != "home")
			{
				if (name == "common.json") return;
				foreach (var s in TextureReplacement.ResidentMenu)
					if (name == s.NxThemeName + ".dds" || name == s.NxThemeName + ".png") return;
			}
			if (info.Target != "lock")
				foreach (var s in TextureReplacement.Entrance)
					if (name == s.NxThemeName + ".dds" || name == s.NxThemeName + ".png") return;
			files.Add(name, data);
		}

		private static (UInt32, UInt32) GetPngSize(byte[] data)
		{
			UInt32 w, h;
			using (BinaryDataReader bin = new BinaryDataReader(new MemoryStream(data)))
			{
				bin.ByteOrder = ByteOrder.BigEndian;
				bin.BaseStream.Position = 0x10;
				w = bin.ReadUInt32();
				h = bin.ReadUInt32();
			}
			return (w, h);
		}

		private static (UInt32, UInt32) GetJpgSize(byte[] data)
		{
			UInt32 w = 0, h = 0;
			using (BinaryDataReader bin = new BinaryDataReader(new MemoryStream(data)))
			{
				bin.ByteOrder = ByteOrder.BigEndian;
				while (bin.BaseStream.Position < bin.BaseStream.Length)
				{
					byte marker = 0;
					while ((marker = bin.ReadByte()) != 0xFF) ;
					while ((marker = bin.ReadByte()) == 0xFF) ;

					if (marker != 0xC0 && marker != 0xC2) continue;
					
					bin.ReadByte();
					bin.ReadByte();
					bin.ReadByte();

					h = bin.ReadUInt16();
					w = bin.ReadUInt16();
				}
			}
			return (w, h);
		}

		public void AddMainBg(byte[] data)
		{
			string ext = "";
			if (data.Matches("DDS "))
			{
				ext = "dds";
				var img = DDSEncoder.LoadDDS(data);
				if (img.width != 1280 || img.height != 720 || img.Format != "DXT1")
					throw new Exception("The background image must be 1280x720 and (if you're using a DDS) DXT1 encoded.");
			}
			else if (data.Matches(6, "JFIF"))
			{
				ext = "jpg";
				(UInt32 w, UInt32 h) = GetJpgSize(data);
				if (w != 1280 || h != 720)
					throw new Exception("The background image must be 1280x720.");
			}
			else throw new Exception("Invalid image format");
			AddFile("image." + ext, data);
		}

		public void AddMainLayout(string text) =>
			AddMainLayout(LayoutPatch.LoadTemplate(text));

		public void AddMainLayout(LayoutPatch l) {
			AddFile("layout.json", l.AsByteArray());
			info.LayoutInfo = l.PatchName + " by " + l.AuthorName;
		}

		public void AddAppletIcon(string name, byte[] data)
		{
			if (!TextureReplacement.NxNameToList.ContainsKey(name)) throw new Exception("Not supported for this target");

			var item = TextureReplacement.NxNameToList[name].Where(x => x.NxThemeName == name).FirstOrDefault();
			if (item == null) throw new Exception($"{name} not supported for this target");

			string ext = "";
			if (data.Matches("DDS "))
			{
				ext = ".dds";
				var img = DDSEncoder.LoadDDS(data);
				if (img.width != item.W || img.height != item.H || (img.Format != "DXT1" && img.Format != "DXT5"))
					throw new Exception("The applet image must be 64x56 and (if you're using a DDS) DXT1/5 encoded.");
			}
			/* TODO: support png for applet images
			else if (data.Matches(1, "PNG"))
			{
				ext = ".png";
				(UInt32 w, UInt32 h) = GetPngSize(data);
				if (w != item.W || h != item.H)
					throw new Exception("The applet image must be 64x56.");
			}*/
			else throw new Exception("Invalid image format");
			AddFile(name + ext, data);
		}
	}

	public class SzsPatcher
	{
		private SarcData sarc;
		private QuickBntx bntx = null;
		private IEnumerable<PatchTemplate> templates;

		public bool EnableAnimations = true;

		public SzsPatcher(SarcData s, IEnumerable<PatchTemplate> t)
		{
			sarc = s;
			templates = t;
		}

		void SaveBntx()
		{
			if (bntx == null) return;
			sarc.Files[@"timg/__Combined.bntx"] = bntx.Write();
			bntx = null;
		}

		QuickBntx GetBntx()
		{
			if (bntx != null) return bntx;
			bntx = new QuickBntx(new BinaryDataReader(new MemoryStream(sarc.Files[@"timg/__Combined.bntx"])));
			return bntx;
		}

		public SarcData GetFinalSarc()
		{
			SaveBntx();
			return sarc;
		}

		public BflytFile.PatchResult PatchAnimations(AnimFilePatch[] files)
		{
			if (files == null) return BflytFile.PatchResult.OK;
			uint TargetVersion = 0;
			foreach (var p in files)
			{
				if (!sarc.Files.ContainsKey(p.FileName))
					continue; //return BflytFile.PatchResult.Fail; Don't be so strict as older firmwares may not have all the animations (?)

				if (TargetVersion == 0)
				{
					Bflan b = new Bflan(sarc.Files[p.FileName]);
					TargetVersion = b.Version;
				}

				var n = BflanSerializer.FromJson(p.AnimJson);
				n.Version = TargetVersion;
				n.byteOrder = Syroot.BinaryData.ByteOrder.LittleEndian;
				sarc.Files[p.FileName] = n.WriteFile();
			}
			return BflytFile.PatchResult.OK;
		}

		private BflytFile.PatchResult PatchSingleLayout(LayoutFilePatch p)
		{
			if (p == null || p.FileName == null) return BflytFile.PatchResult.OK;
			if (!sarc.Files.ContainsKey(p.FileName))
				return BflytFile.PatchResult.Fail;
			var target = new BflytFile(sarc.Files[p.FileName]);
			target.ApplyMaterialsPatch(p.Materials); //Do not check result as it fails only if the file doesn't have any material
			var res = target.ApplyLayoutPatch(p.Patches);
			if (res != BflytFile.PatchResult.OK)
				return res;
			if (EnableAnimations)
			{
				res = target.AddGroupNames(p.AddGroups);
				if (res != BflytFile.PatchResult.OK)
					return res;
			}
			sarc.Files[p.FileName] = target.SaveFile();
			return BflytFile.PatchResult.OK;
		}

		public BflytFile.PatchResult PatchLayouts(LayoutPatch Patch, string PartName, bool FixFor8)
		{
			if (PartName == "home" && Patch.PatchAppletColorAttrib)
				PatchBntxTextureAttribs(new Tuple<string, uint>("RdtIcoPvr_00^s", 0x02000000),
				   new Tuple<string, uint>("RdtIcoNews_00^s", 0x02000000), new Tuple<string, uint>("RdtIcoNews_01^s", 0x02000000),
				   new Tuple<string, uint>("RdtIcoSet^s", 0x02000000), new Tuple<string, uint>("RdtIcoShop^s", 0x02000000),
				   new Tuple<string, uint>("RdtIcoCtrl_00^s", 0x02000000), new Tuple<string, uint>("RdtIcoCtrl_01^s", 0x02000000),
				   new Tuple<string, uint>("RdtIcoCtrl_02^s", 0x02000000), new Tuple<string, uint>("RdtIcoPwrForm^s", 0x02000000));

			List<LayoutFilePatch> Files = new List<LayoutFilePatch>();
			Files.AddRange(Patch.Files);
			if (FixFor8 && !Patch.Ready8X)
			{
				var extra = NewFirmFixes.GetFix(Patch.PatchName);
				if (extra != null)
					Files.AddRange(extra);
			}

			foreach (var p in Files)
			{
				var res = PatchSingleLayout(p);
				if (res != BflytFile.PatchResult.OK) return res;
			}
			return BflytFile.PatchResult.OK;
		}

		public BflytFile.PatchResult PatchBntxTexture(byte[] DDS, string texName, uint TexFlag = 0xFFFFFFFF)
		{
			QuickBntx q = GetBntx();
			if (q.Rlt.Length != 0x80)
			{
				return BflytFile.PatchResult.CorruptedFile;
			}
			q.ReplaceTex(texName, DDS);
			if (TexFlag != 0xFFFFFFFF)
				q.FindTex(texName).ChannelTypes = (int)TexFlag;
			return BflytFile.PatchResult.OK;
		}

		public BflytFile.PatchResult PatchAppletIcon(byte[] DDS, string name)
		{
			var patch = DetectSarc();
			if (!TextureReplacement.NxNameToList.ContainsKey(patch.NXThemeName))
				return BflytFile.PatchResult.Fail;

			var target = TextureReplacement.NxNameToList[patch.NXThemeName].Where(x => x.NxThemeName == name).First();

			var res = PatchSingleLayout(target.patch);
			if (res != BflytFile.PatchResult.OK) return res;

			PatchBntxTexture(DDS, target.BntxName, target.NewColorFlags);

			BflytFile curTarget = new BflytFile(sarc.Files[target.FileName]);
			curTarget.ClearUVData(target.PaneName);
			sarc.Files[target.FileName] = curTarget.SaveFile();

			return BflytFile.PatchResult.OK;
		}


		public BflytFile.PatchResult PatchMainBG(byte[] DDS)
		{
			return PatchMainBG(DDSEncoder.LoadDDS(DDS));
		}

		public BflytFile.PatchResult PatchMainBG(DDSEncoder.DDSLoadResult DDS)
		{
			var template = DetectSarc();
			BflytFile BflytFromSzs(string name) => new BflytFile(sarc.Files[name]);

			//PatchBGLayouts
			BflytFile MainFile = BflytFromSzs(template.MainLayoutName);
			var res = MainFile.PatchBgLayout(template);
			if (res == BflytFile.PatchResult.CorruptedFile || res == BflytFile.PatchResult.Fail)
				return res;

			sarc.Files[template.MainLayoutName] = MainFile.SaveFile();
			var layouts = sarc.Files.Keys.Where(x => x.StartsWith("blyt/") && x.EndsWith(".bflyt") && x != template.MainLayoutName).ToArray();
			foreach (var f in layouts)
			{
				BflytFile curTarget = BflytFromSzs(f);
				if (curTarget.PatchTextureName(template.MaintextureName, template.SecondaryTexReplace))
					sarc.Files[f] = curTarget.SaveFile();
			}

			//PatchBGBntx
			QuickBntx q = GetBntx();
			if (q.Rlt.Length != 0x80)
			{
				return BflytFile.PatchResult.CorruptedFile;
			}
			q.ReplaceTex(template.MaintextureName, DDS);
			DDS = null;
			return BflytFile.PatchResult.OK;
		}

		public BflytFile.PatchResult PatchBntxTextureAttribs(params Tuple<string, UInt32>[] patches)
		{
			QuickBntx q = GetBntx();
			if (q.Rlt.Length != 0x80)
			{
				return BflytFile.PatchResult.CorruptedFile;
			}
			try
			{
				foreach (var patch in patches)
				{
					var target = q.FindTex(patch.Item1);
					if (target != null) target.ChannelTypes = (int)patch.Item2;
				}
			}
			catch (Exception ex)
			{
				return BflytFile.PatchResult.Fail;
			}
			return BflytFile.PatchResult.OK;
		}

		public PatchTemplate DetectSarc()
		{
			return DetectSarc(sarc, templates);
		}

		public static PatchTemplate DetectSarc(SarcData sarc, IEnumerable<PatchTemplate> Templates)
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
	}
}
