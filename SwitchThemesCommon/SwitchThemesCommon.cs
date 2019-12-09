using SwitchThemes.Common.Bntxx;
using SwitchThemes.Common.Bflyt;
using SwitchThemes.Common.Bflan;
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
		public const string CoreVer = "4.3.3";
		public const int NxThemeFormatVersion = 10;

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
			if (!files.ContainsKey("image.dds") && !files.ContainsKey("image.jpg") && !files.ContainsKey("layout.json"))
				throw new Exception("An nxtheme must contain at least a custom background image or layout");

			if (!files.ContainsKey("info.json"))
				AddFile("info.json", Encoding.UTF8.GetBytes(info.Serialize()));

			var sarc = SARCExt.SARC.PackN(new SARCExt.SarcData() { endianness = ByteOrder.LittleEndian, Files = files, HashOnly = false });
#if WIN
			return ManagedYaz0.Compress(sarc.Item2, 1, (int)sarc.Item1);
#else
			return ManagedYaz0.Compress(sarc.Item2, 0, (int)sarc.Item1);
#endif
		}

		public void AddFile(string name, byte[] data)
		{
			if (name == null || data == null) return;
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

		private static (UInt32, UInt32, bool) GetJpgInfo(byte[] data) //width, height, progressive
		{
			UInt32 w = 0, h = 0;
			bool Progressive = false;
			using (BinaryDataReader bin = new BinaryDataReader(new MemoryStream(data)))
			{
				bin.ByteOrder = ByteOrder.BigEndian;
				while (bin.BaseStream.Position < bin.BaseStream.Length)
				{
					byte marker = 0;
					while ((marker = bin.ReadByte()) != 0xFF) ;
					while ((marker = bin.ReadByte()) == 0xFF) ;

					if (marker == 0xC0)
					{

						bin.ReadByte();
						bin.ReadByte();
						bin.ReadByte();

						h = bin.ReadUInt16();
						w = bin.ReadUInt16();
					}
					if (marker == 0xC2)
					{
						Progressive = true;
					}
				}
			}
			return (w, h, Progressive);
		}

		public void AddMainBg(byte[] data)
		{
			if (data == null) return;
			string ext = "";
			if (data.Matches("DDS "))
			{
				ext = "dds";
				var img = DDSEncoder.LoadDDS(data);
				if (img.width != 1280 || img.height != 720 || img.Format != "DXT1")
					throw new Exception("The background image must be 1280x720 and (if you're using a DDS) DXT1 encoded.");
			}
			else if (data.Matches(0,new byte[] { 0xFF, 0xD8, 0xFF }))
			{
				ext = "jpg";
				(UInt32 w, UInt32 h, bool IsProgressive) = GetJpgInfo(data);

				if (IsProgressive)
					throw new Exception("Progressive JPG images are not currently supported for the background image, check the encoding settings in your image editor");

				if (w != 1280 || h != 720)
					throw new Exception("The background image must be 1280x720.");
			}
			else throw new Exception("Invalid image format: The background image can only be a DDS or JPG image");
			AddFile("image." + ext, data);
		}

		public void AddMainLayout(string text) =>
			AddMainLayout(LayoutPatch.LoadTemplate(text));

		public void AddMainLayout(LayoutPatch l) {
			if (l == null) return;
			AddFile("layout.json", l.AsByteArray());
			info.LayoutInfo = l.PatchName + " by " + l.AuthorName;
		}

		public void AddAppletIcon(string name, byte[] data)
		{
			if (!TextureReplacement.NxNameToList.ContainsKey(info.Target)) throw new Exception("Not supported for this target");

			var item = TextureReplacement.NxNameToList[info.Target].Where(x => x.NxThemeName == name).FirstOrDefault();
			if (item == null) throw new Exception($"{name} not supported for this target");

			string ext = "";
			if (data.Matches("DDS "))
			{
				ext = ".dds";
				var img = DDSEncoder.LoadDDS(data);
				if (img.width != item.W || img.height != item.H || (img.Format != "DXT1" && img.Format != "DXT4" && img.Format != "DXT5" && img.Format != "DXT3"))
					throw new Exception("The applet image must be 64x56 and (if you're using a DDS) DXT1/3/4/5 encoded.");
			}
			else if (data.Matches(1, "PNG"))
			{
				ext = ".png";
				(UInt32 w, UInt32 h) = GetPngSize(data);
				if (w != item.W || h != item.H)
					throw new Exception("The applet image must be 64x56.");
			}
			else throw new Exception("Invalid image format: Applet icons can only be DDS or PNG images");
			AddFile(name + ext, data);
		}
	}

	public class SzsPatcher
	{
		private SarcData sarc;
		private QuickBntx bntx = null;
		private IEnumerable<PatchTemplate> templates;

		public bool EnableAnimations = true;
		public bool EnablePaneOrderMod = true;

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

		public bool PatchAnimations(AnimFilePatch[] files)
		{
			if (files == null) return true;
			uint TargetVersion = 0;
			foreach (var p in files)
			{
				if (!sarc.Files.ContainsKey(p.FileName))
					continue; //return bool.Fail; Don't be so strict as older firmwares may not have all the animations (?)

				if (TargetVersion == 0)
				{
					BflanFile b = new BflanFile(sarc.Files[p.FileName]);
					TargetVersion = b.Version;
				}

				var n = BflanSerializer.FromJson(p.AnimJson);
				n.Version = TargetVersion;
				n.byteOrder = Syroot.BinaryData.ByteOrder.LittleEndian;
				sarc.Files[p.FileName] = n.WriteFile();
			}
			return true;
		}

		private bool PatchSingleLayout(LayoutFilePatch p)
		{
			if (p == null || p.FileName == null) return true;
			if (!sarc.Files.ContainsKey(p.FileName))
				return false;
			var target = new BflytFile(sarc.Files[p.FileName]);
			target.ApplyMaterialsPatch(p.Materials); //Do not check result as it fails only if the file doesn't have any material
			var res = target.ApplyLayoutPatch(p.Patches);
			if (!res) return res;
			if (EnableAnimations)
			{
				res = target.AddGroupNames(p.AddGroups);
				if (!res) return res;
			}

			if (p.PullFrontPanes != null)
				foreach (var n in p.PullFrontPanes)
					target.PanePullToFront(n);
			if (p.PushBackPanes != null)
				foreach (var n in p.PushBackPanes)
					target.PanePushBack(n);

			sarc.Files[p.FileName] = target.SaveFile();
			return true;
		}

		public bool PatchLayouts(LayoutPatch Patch, string PartName, PatchTemplate context)
		{
			if (PartName == "home" && Patch.PatchAppletColorAttrib)
				PatchBntxTextureAttribs(new Tuple<string, uint>("RdtIcoPvr_00^s", 0x5050505),
				   new Tuple<string, uint>("RdtIcoNews_00^s", 0x5050505), new Tuple<string, uint>("RdtIcoNews_01^s", 0x5050505),
				   new Tuple<string, uint>("RdtIcoSet^s", 0x5050505), new Tuple<string, uint>("RdtIcoShop^s", 0x5050505),
				   new Tuple<string, uint>("RdtIcoCtrl_00^s", 0x5050505), new Tuple<string, uint>("RdtIcoCtrl_01^s", 0x5050505),
				   new Tuple<string, uint>("RdtIcoCtrl_02^s", 0x5050505), new Tuple<string, uint>("RdtIcoPwrForm^s", 0x5050505));

			List<LayoutFilePatch> Files = new List<LayoutFilePatch>();
			Files.AddRange(Patch.Files);

			int fixVer = 0;
			if (context != null)
			{
				if (context.NXThemeName == "home")
					fixVer = 8;
				if (context.FirmName == "9.0")
					fixVer = 9;
			}
			if (fixVer >= 8 && !Patch.Ready8X)
			{
				var extra = NewFirmFixes.GetFix(Patch.PatchName, context);
				if (extra != null)
					Files.AddRange(extra);
			}

			foreach (var p in Files)
			{
				var res = PatchSingleLayout(p);
				if (!res) return res;
			}
			return true;
		}

		public bool PatchBntxTexture(byte[] DDS, string texName, uint TexFlag = 0xFFFFFFFF)
		{
			QuickBntx q = GetBntx();
			if (q.Rlt.Length != 0x80)
				return false;
			q.ReplaceTex(texName, DDS);
			if (TexFlag != 0xFFFFFFFF)
				q.FindTex(texName).ChannelTypes = (int)TexFlag;
			return true;
		}

		public bool PatchAppletIcon(byte[] DDS, string name)
		{
			var patch = PatchTemplate;
			if (!TextureReplacement.NxNameToList.ContainsKey(patch.NXThemeName))
				return false;

			var target = TextureReplacement.NxNameToList[patch.NXThemeName].Where(x => x.NxThemeName == name).First();

			var res = PatchSingleLayout(target.patch);
			if (!res) return res;

			PatchBntxTexture(DDS, target.BntxName, target.NewColorFlags);

			BflytFile curTarget = new BflytFile(sarc.Files[target.FileName]);
			curTarget.ClearUVData(target.PaneName);
			sarc.Files[target.FileName] = curTarget.SaveFile();

			return true;
		}


		public bool PatchMainBG(byte[] DDS)
		{
			return PatchMainBG(DDSEncoder.LoadDDS(DDS));
		}

		public bool PatchMainBG(DDSEncoder.DDSLoadResult DDS)
		{
			var template = PatchTemplate;
			BflytFile BflytFromSzs(string name) => new BflytFile(sarc.Files[name]);

			//PatchBGLayouts
			BflytFile MainFile = BflytFromSzs(template.MainLayoutName);
			var res = MainFile.PatchBgLayout(template);
			if (!res) return res;

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
				return false;
			q.ReplaceTex(template.MaintextureName, DDS);
			DDS = null;
			return true;
		}

		public bool PatchBntxTextureAttribs(params Tuple<string, UInt32>[] patches)
		{
			QuickBntx q = GetBntx();
			if (q.Rlt.Length != 0x80)
				return false;
			try
			{
				foreach (var patch in patches)
				{
					var target = q.FindTex(patch.Item1);
					if (target != null) target.ChannelTypes = (int)patch.Item2;
				}
			}
			catch
			{
				return false;
			}
			return true;
		}

		private PatchTemplate _patch = null;
		public PatchTemplate PatchTemplate
		{
			get
			{
				if (_patch != null) return _patch;
				_patch = DetectSarc(sarc, templates);
				return _patch;
			}
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
