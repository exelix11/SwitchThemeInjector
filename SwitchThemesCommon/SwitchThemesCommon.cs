using SwitchThemes.Common.Bntxx;
using SwitchThemes.Common.Bflyt;
using SwitchThemes.Common.Bflan;
using SwitchThemes.Common.Serializers;
using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SARCExt;
using System.IO;

namespace SwitchThemes.Common
{
    public static class Info
    {
        public const string CoreVer = "4.8.3";
        public const int NxThemeFormatVersion = 16;

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

    public enum LayoutCompatibilityOption 
    {
        // Layout fixes will be applied automatically using our heuristics and version detection
        Default,
        // Disable all layout fixes
        DisableFixes,
        // Forces pre-11.0 layout by removing the new applet icons, keeping only the stock release ones
        Firmware10,
        // Forces the 11.0 layout by removing all the applet icons added with 20.0 and keeping only the stock + NS online one
        Firmware11
    }

    public class SzsPatcher
    {
        readonly SarcData Sarc;
        public readonly ConsoleFirmware TargetFirmware;
        public readonly PatchTemplate PatchTemplate;

        public LayoutCompatibilityOption CompatFixes = LayoutCompatibilityOption.Default;

        private QuickBntx bntx = null;

        public SzsPatcher(SarcData s)
        {
            Sarc = s;
            PatchTemplate = DefaultTemplates.GetFor(Sarc);

            if (PatchTemplate != null)
                TargetFirmware = FirmwareDetection.Detect(PatchTemplate.NXThemeName, Sarc);
            else
                TargetFirmware = ConsoleFirmware.Invariant;
        }

        void SaveBntx()
        {
            if (bntx == null) return;
            Sarc.Files[@"timg/__Combined.bntx"] = bntx.Write();
            bntx = null;
        }

        QuickBntx GetBntx()
        {
            if (bntx != null) return bntx;
            bntx = new QuickBntx(Sarc.Files[@"timg/__Combined.bntx"]);
            return bntx;
        }

        public SarcData GetFinalSarc()
        {
            SaveBntx();
            return Sarc;
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
            if (!Sarc.Files.ContainsKey(p.FileName))
                return false;

            // The bflan version varies between firmwares, load a file from the current firmware to get the target version
            // cache this result to avoid loading all files
            if (!FirmwareTargetBflanVersion.HasValue)
            {
                BflanFile b = new BflanFile(Sarc.Files[p.FileName]);
                FirmwareTargetBflanVersion = b.Version;
            }

            var n = BflanSerializer.FromJson(p.AnimJson);
            n.Version = FirmwareTargetBflanVersion.Value;
            n.byteOrder = Syroot.BinaryData.ByteOrder.LittleEndian;
            Sarc.Files[p.FileName] = n.WriteFile();

            return true;
        }

        private bool ApplyLayoutPatch(LayoutFilePatch p)
        {
            if (p == null || p.FileName == null) return true;
            if (!Sarc.Files.ContainsKey(p.FileName))
                return false;

            var target = new BflytFile(Sarc.Files[p.FileName]);

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

            Sarc.Files[p.FileName] = target.SaveFile();
            return true;
        }

        public bool PatchLayouts(LayoutPatch Patch) =>
            PatchLayouts(Patch, PatchTemplate?.NXThemeName ?? "");

        // Hacky. see my comment on https://github.com/exelix11/SwitchThemeInjector/issues/156#issuecomment-2869845256
        // Some layout are broken on 20.0 because they have animations that reference missing panes
        // As an extreme workaround we remove all the animations that cause the crash
        // This method will remove any such animation from the json
        // Currently we don't apply checks on the bflyt patches since we automatically ignore the ones that don't exist anymore
        private int FilterIncompatibleAnimations(LayoutPatch layout)
        {
            if (layout.Anims is null)
                return 0;

            var remove = new HashSet<string>();

            var list = new List<LayoutCompatibility.CompatIssue>();
            foreach (var anim in layout.Anims)
            {
                list.Clear();
                var parsed = BflanSerializer.FromJson(anim.AnimJson);
                LayoutCompatibility.CheckAnimationCompatibility(list, layout, Sarc, anim.FileName, parsed);

                // TODO: Should unknown files be treated as errors ?
                if (list.Any(x => x.Severity == LayoutCompatibility.ProblemSeverity.Critical))
                    remove.Add(anim.FileName);
            }

            // Remove all the animations that caused issues
            layout.Anims = layout.Anims
                .Where(x => !remove.Contains(x.FileName))
                .ToArray();

            return remove.Count;
        }

        private bool PatchLayouts(LayoutPatch Patch, string PartName)
        {
            // Compatibility flags
            bool useLegacyFixes = false;
            bool useModernFixes = false;
            bool appletPositionFixes = false;
            bool onlineBtnFix = false;

            if (CompatFixes == LayoutCompatibilityOption.Firmware10 && PartName == "home")
                Patch.HideOnlineBtn = true;

            if (CompatFixes == LayoutCompatibilityOption.Firmware11 && PartName == "home")
            {
                Patch.HideOnlineBtn = false;
                Patch.TargetFirmwareValue = ConsoleFirmware.Fw11_0;
            }

            if (CompatFixes != LayoutCompatibilityOption.DisableFixes)
            {
                // Detect any compatibility patches we need
                useLegacyFixes = TargetFirmware != ConsoleFirmware.Invariant && Patch.UsesOldFixes;
                useModernFixes = !useLegacyFixes && Patch.ID != null;
                appletPositionFixes = PartName == "home" && NewFirmFixes.ShouldApplyAppletPositionFix(Patch, TargetFirmware);

                // The default for this on old layouts that don't specify it is true
                onlineBtnFix = PartName == "home" && (Patch.HideOnlineBtn ?? true);
            }

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
                ApplyRawPatch(NewFirmFixes.GetAppletsPositionFix(TargetFirmware));

            if (onlineBtnFix)
                ApplyRawPatch(NewFirmFixes.GetLegacyAppletButtonsFix(TargetFirmware));

            // GetFix might modify the layout to make it compatible.
            // So while its result must be applied as an overlay we must call it before applying the patch.
            LayoutPatch modernFix = null;
            if (useModernFixes)
                modernFix = NewFirmFixes.GetFix(Patch, TargetFirmware);

            if (CompatFixes != LayoutCompatibilityOption.DisableFixes)
                FilterIncompatibleAnimations(Patch);

            // Then json patches
            ApplyRawPatch(Patch);

            // Then fixes on top of known broken layouts
            if (useLegacyFixes)
                ApplyRawPatch(NewFirmFixes.GetFixLegacy(Patch.PatchName, TargetFirmware, PartName));

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

            var target = TextureReplacement.NxNameToList[patch.NXThemeName]
                .Where(x => x.NxThemeName == name).FirstOrDefault();

            if (target == null)
                return false;

            // THis applet icon is not present in the current firmware. Nothing to do.
            if (TargetFirmware < target.MinFirmware)
                return true;

            if (!ApplyLayoutPatch(target.Patch))
                return false;

            PatchBntxTexture(DDS, target.BntxNames, target.NewColorFlags);

            BflytFile curTarget = new BflytFile(Sarc.Files[target.FileName]);
            curTarget.ClearUVData(target.PaneName);
            Sarc.Files[target.FileName] = curTarget.SaveFile();

            return true;
        }

        public bool PatchMainBG(byte[] DDS)
        {
            return PatchMainBG(new Images.DDS(DDS));
        }

        public bool PatchMainBG(Images.DDS DDS)
        {
            var template = PatchTemplate;
            BflytFile BflytFromSzs(string name) => new BflytFile(Sarc.Files[name]);

            //PatchBGLayouts
            BflytFile MainFile = BflytFromSzs(template.MainLayoutName);
            var res = MainFile.PatchBgLayout(template);
            if (!res) return res;

            Sarc.Files[template.MainLayoutName] = MainFile.SaveFile();

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

            var layouts = Sarc.Files.Keys.Where(x => x.StartsWith("blyt/") && x.EndsWith(".bflyt") && x != template.MainLayoutName).ToArray();
            foreach (var f in layouts)
            {
                BflytFile curTarget = BflytFromSzs(f);
                if (curTarget.PatchTextureName(template.MaintextureName, replaceWith))
                    Sarc.Files[f] = curTarget.SaveFile();
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
    }
}
