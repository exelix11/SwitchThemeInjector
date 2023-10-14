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
	public static class Info
	{
		public const string CoreVer = "4.7.1";
		public const int NxThemeFormatVersion = 15;

		public static Dictionary<string, string> PartToFileName = new Dictionary<string, string>() {
			{"home", "ResidentMenu.szs"},
			{"lock", "Entrance.szs"},
			{"user", "MyPage.szs"},
			{"apps", "Flaunch.szs"},
			{"set" , "Set.szs"},
			{"news", "Notification.szs"},
			{"psl" , "Psl.szs" },
		};

		public static Dictionary<string, string> PartToName = new Dictionary<string, string>()
		{
			{"home", "Home menu" },
			{"lock", "Lock screen" },
			{"user", "User page" },
			{"apps", "All apps menu (All applets on 5.X)" },
			{"set" , "Settings applet (All applets on 5.X)" },
			{"news", "News applet (All applets on 5.X)" },
			{"psl" , "Player select" },
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
				Version = Info.NxThemeFormatVersion,
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

			var sarc = SARCExt.SARC.Pack(new SARCExt.SarcData() { endianness = ByteOrder.LittleEndian, Files = files, HashOnly = false });
#if WIN
			return ManagedYaz0.Compress(sarc.Item2, 3, (int)sarc.Item1);
#else
			return ManagedYaz0.Compress(sarc.Item2, 0, (int)sarc.Item1);
#endif
		}

		private void AddFile(string name, byte[] data)
		{
			if (name == null || data == null)
				return;

			if (info.Target != "home" && name == "common.json")
				return;

			files.Add(name, data);
		}

		public void AddCommonLayout(string json) =>
			AddFile("common.json", LayoutPatch.Load(json).AsByteArray());

		public void AddCommonLayout(LayoutPatch data) =>
			AddFile("common.json", data.AsByteArray());

		public void AddMainBg(byte[] data)
		{
			if (data == null) return;
			var fmt = Images.Validation.AssertValidForBG(data);
			AddFile("image." + fmt.Extension, data);
		}

		public void AddMainLayout(string text) =>
			AddMainLayout(LayoutPatch.Load(text));

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

			var img = Images.Validation.AssertValidForApplet(item, data);

			AddFile($"{name}.{img.Extension}", data);
		}
	}

	public class SzsPatcher
	{
		private SarcData sarc;
		private QuickBntx bntx = null;

		public bool EnablePaneOrderMod = true;

		public SzsPatcher(SarcData s) 
		{
			sarc = s;
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
			bntx = new QuickBntx(sarc.Files[@"timg/__Combined.bntx"]);
			return bntx;
		}

		public SarcData GetFinalSarc()
		{
			SaveBntx();
			return sarc;
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

			res = target.AddGroupNames(p.AddGroups);
			if (!res) return res;

			if (p.PullFrontPanes != null)
				foreach (var n in p.PullFrontPanes)
					target.PanePullToFront(n);
			if (p.PushBackPanes != null)
				foreach (var n in p.PushBackPanes)
					target.PanePushBack(n);

			sarc.Files[p.FileName] = target.SaveFile();
			return true;
		}

		public bool PatchLayouts(LayoutPatch Patch) =>
			PatchLayouts(Patch, PatchTemplate?.NXThemeName ?? "");
		
		private bool PatchLayouts(LayoutPatch Patch, string PartName)
		{
			var fw = FirmwareDetection.Detect(PartName, sarc);

			if (PartName == "home" && Patch.PatchAppletColorAttrib)
				PatchBntxTextureAttribs(new Tuple<string, uint>("RdtIcoPvr_00^s", 0x5050505),
				   new Tuple<string, uint>("RdtIcoNews_00^s", 0x5050505), new Tuple<string, uint>("RdtIcoNews_01^s", 0x5050505),
				   new Tuple<string, uint>("RdtIcoSet^s", 0x5050505), new Tuple<string, uint>("RdtIcoShop^s", 0x5050505),
				   new Tuple<string, uint>("RdtIcoCtrl_00^s", 0x5050505), new Tuple<string, uint>("RdtIcoCtrl_01^s", 0x5050505),
				   new Tuple<string, uint>("RdtIcoCtrl_02^s", 0x5050505), new Tuple<string, uint>("RdtIcoPwrForm^s", 0x5050505));

			List<LayoutFilePatch> Files = new List<LayoutFilePatch>();
			Files.AddRange(Patch.Files);

			LayoutFilePatch[] extra;
			//Legacy fixes based on name and version
			if (fw != FirmwareDetection.Firmware.Invariant && Patch.UsesOldFixes)
			{
				extra = NewFirmFixes.GetFixLegacy(Patch.PatchName, fw, PartName);
				if (extra != null)
					Files.AddRange(extra);
			}
			//Modern fixes based on layout ID
			else if (Patch.ID != null)
			{
				extra = NewFirmFixes.GetFix(PartName, Patch.ID, fw);
				if (extra != null)
					Files.AddRange(extra);
			}

			foreach (var p in Files)
			{
				var res = PatchSingleLayout(p);
				if (!res) return res;
			}

			List<AnimFilePatch> Anims = new List<AnimFilePatch>();
			if (Patch.Anims != null)
				Anims.AddRange(Patch.Anims);

			if (PartName == "home")
			{
				AnimFilePatch[] animExtra = null;
				if (Patch.HideOnlineBtn ?? true)
					animExtra = NewFirmFixes.GetNoOnlineButtonFix(fw);
				else if (NewFirmFixes.ShouldApplyAppletPositionFix(Anims))
					animExtra = NewFirmFixes.GetAppletsPositionFix(fw);

				if (animExtra != null)
					Anims.AddRange(animExtra);
			}

			if (Anims.Any())
			{
				// The bflan version varies between firmwares, load a file from the list to detect the right one
				BflanFile b = new BflanFile(sarc.Files[Anims[0].FileName]);
				var TargetVersion = b.Version;
				b = null;

				foreach (var p in Anims)
				{
					if (!sarc.Files.ContainsKey(p.FileName))
						continue;

					var n = BflanSerializer.FromJson(p.AnimJson);
					n.Version = TargetVersion;
					n.byteOrder = Syroot.BinaryData.ByteOrder.LittleEndian;
					sarc.Files[p.FileName] = n.WriteFile();
				}
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
			return PatchMainBG(new Images.DDS(DDS));
		}

		public bool PatchMainBG(Images.DDS DDS)
		{
			var template = PatchTemplate;
			BflytFile BflytFromSzs(string name) => new BflytFile(sarc.Files[name]);

			//PatchBGLayouts
			BflytFile MainFile = BflytFromSzs(template.MainLayoutName);
			var res = MainFile.PatchBgLayout(template);
			if (!res) return res;

			sarc.Files[template.MainLayoutName] = MainFile.SaveFile();

			//PatchBGBntx
			QuickBntx q = GetBntx();
			if (q.Rlt.Length != 0x80)
				return false;
			q.ReplaceTex(template.MaintextureName, DDS);

            // Remove references to the texture we replaced from other layouts

			// If the hardcoded texture is not present fallback to the first one called White*
            var replaceWith = 
				q.Textures.Any(x => x.Name == template.SecondaryTexReplace) ? template.SecondaryTexReplace :
				q.Textures.FirstOrDefault(x => x.Name.StartsWith("White"))?.Name;

			if (replaceWith == null)
				return false;
            
            var layouts = sarc.Files.Keys.Where(x => x.StartsWith("blyt/") && x.EndsWith(".bflyt") && x != template.MainLayoutName).ToArray();
			foreach (var f in layouts)
			{
				BflytFile curTarget = BflytFromSzs(f);
				if (curTarget.PatchTextureName(template.MaintextureName, replaceWith))
					sarc.Files[f] = curTarget.SaveFile();
			}

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
				_patch = DefaultTemplates.GetFor(sarc);
				return _patch;
			}
		}
	}
}
