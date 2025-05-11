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
using System.Text.RegularExpressions;

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

        public void AddMainLayout(LayoutPatch l)
        {
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

        private void ApplyRawPatch(LayoutPatch patch)
        {
            // Note that we ignore non-critical errors such as missing files or panes to improve compatibility across multiple firmware
            // Critical errors will throw an exception and stop the patching process

            if (patch == null)
                return;

            if (patch.Files != null) foreach (var p in patch.Files) ApplyLayoutPatch(p);
            if (patch.Anims != null) foreach (var p in patch.Anims) ApplyAnimPatch(p);
        }

        uint? FirmwareTargetBflanVersion = null;
        private bool ApplyAnimPatch(AnimFilePatch p)
        {
            if (!sarc.Files.ContainsKey(p.FileName))
                return false;

            // The bflan version varies between firmwares, load a file from the current firmware to get the target version
            // cache this result to avoid loading all files
            if (!FirmwareTargetBflanVersion.HasValue)
            {
                BflanFile b = new BflanFile(sarc.Files[p.FileName]);
                FirmwareTargetBflanVersion = b.Version;
            }

            var n = BflanSerializer.FromJson(p.AnimJson);
            n.Version = FirmwareTargetBflanVersion.Value;
            n.byteOrder = Syroot.BinaryData.ByteOrder.LittleEndian;
            sarc.Files[p.FileName] = n.WriteFile();

            return true;
        }

        private bool ApplyLayoutPatch(LayoutFilePatch p)
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

            // Detect any compatibility patches we need
            var useLegacyFixes = fw != ConsoleFirmware.Invariant && Patch.UsesOldFixes;
            var useModernFixes = !useLegacyFixes && Patch.ID != null;
            var appletPositionFixes = PartName == "home" && NewFirmFixes.ShouldApplyAppletPositionFix(Patch, fw);
            // The default for this on old layouts that don't specify it is true
            var onlineBtnFix = PartName == "home" && (Patch.HideOnlineBtn ?? true);

            // Apply legacy PatchAppletColorAttrib patch
            if (PartName == "home" && Patch.PatchAppletColorAttrib)
                PatchBntxTextureAttribs(new Tuple<string, uint>("RdtIcoPvr_00^s", 0x5050505),
                   // News icon before 20.0.0:
                   new Tuple<string, uint>("RdtIcoNews_00^s", 0x5050505), new Tuple<string, uint>("RdtIcoNews_01^s", 0x5050505),
                   // news icon for 20.0.0 and later:
                   new Tuple<string, uint>("RdtIcoNews_00_Home^s", 0x5050505), new Tuple<string, uint>("RdtIcoNews_01_Home^s", 0x5050505),
                   new Tuple<string, uint>("RdtIcoSet^s", 0x5050505), new Tuple<string, uint>("RdtIcoShop^s", 0x5050505),
                   new Tuple<string, uint>("RdtIcoCtrl_00^s", 0x5050505), new Tuple<string, uint>("RdtIcoCtrl_01^s", 0x5050505),
                   new Tuple<string, uint>("RdtIcoCtrl_02^s", 0x5050505), new Tuple<string, uint>("RdtIcoPwrForm^s", 0x5050505));

            // Apply patches. The order here matters.

            // First home menu fixes. These are applied early so later patches from the json can override them
            if (appletPositionFixes)
                ApplyRawPatch(NewFirmFixes.GetAppletsPositionFix(fw));

            if (onlineBtnFix)
                ApplyRawPatch(NewFirmFixes.GetLegacyAppletButtonsFix(fw));

            // GetFix might modify the layout to make it compatible.
            // So while its result must be applied as an overlay we must call it before applying the patch.
            LayoutPatch modernFix = null;
            if (useModernFixes)
                modernFix = NewFirmFixes.GetFix(Patch, fw);

            // Then json patches
            ApplyRawPatch(Patch);

            // Then fixes on top of known broken layouts
            if (useLegacyFixes)
                ApplyRawPatch(NewFirmFixes.GetFixLegacy(Patch.PatchName, fw, PartName));

            if (useModernFixes)
                ApplyRawPatch(modernFix);

            return true;
        }

        public bool PatchBntxTexture(byte[] DDS, string[] texNames, uint TexFlag = 0xFFFFFFFF)
        {
            QuickBntx q = GetBntx();
            if (q.Rlt.Length != 0x80)
                return false;
            // Replace the first texture whose name in the list is present.
            foreach (var texName in texNames)
            {
                var texture = q.FindTex(texName);
                if (texture == null) continue;

                q.ReplaceTex(texName, DDS);
                if (TexFlag != 0xFFFFFFFF)
                    q.FindTex(texName).ChannelTypes = (int)TexFlag;
                return true;
            }
            return false;
        }

        public bool PatchAppletIcon(byte[] DDS, string name)
        {
            var patch = PatchTemplate;
            if (!TextureReplacement.NxNameToList.ContainsKey(patch.NXThemeName))
                return false;

            var target = TextureReplacement.NxNameToList[patch.NXThemeName].Where(x => x.NxThemeName == name).First();

            var res = ApplyLayoutPatch(target.patch);
            if (!res) return res;

            PatchBntxTexture(DDS, target.BntxNames, target.NewColorFlags);

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
