using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.Text;

namespace SwitchThemes.Common
{
    public class PatchPartInfo
    {
        public readonly string Name;
        public readonly string Description;
        public readonly string SzsName;

        public readonly string TitleId;
        
        public readonly bool AllowLayout;
        public readonly bool AllowImages;

        public PatchPartInfo(string name, string description, string szsName, 
            string titleId = "0100000000001000", 
            bool allowLayout = true,
            bool allowImages = true)
        {
            Name = name;
            Description = description;
            SzsName = szsName;
            TitleId = titleId;
            AllowLayout = allowLayout;
            AllowImages = allowImages;
        }
    }

    public static class CommonInfo
    {
        public const string CoreVer = "5.0";
        public const int NxThemeFormatVersion = 16;
        public const int NxTheme2FormatVersion = 17;

        public static List<PatchPartInfo> Parts = new List<PatchPartInfo>()
        {
            new PatchPartInfo("home", "ResidentMenu.szs", "Home menu"),
            new PatchPartInfo("lock", "Entrance.szs", "Lock screen"),
            new PatchPartInfo("apps", "Flaunch.szs", "Player select"),
            new PatchPartInfo("set" , "Set.szs", "All apps menu"),
            new PatchPartInfo("news", "Notification.szs", "Settings applet"),
            new PatchPartInfo("qlaunch_common", "commmon.szs", "Common layout properties", allowImages: false),
            new PatchPartInfo("user", "MyPage.szs", "User page", "0100000000001013"),
            new PatchPartInfo("psl", "Psl.szs" , "News applet", "0100000000001007"),
        };

        public static PatchPartInfo GetPart(string name) =>
            Parts.Where(x => x.Name == name).FirstOrDefault();
    }

    public class NXThemeBuilder
    {
        private Dictionary<string, byte[]> files = new Dictionary<string, byte[]>();
        ThemeFileManifest info;

        public NXThemeBuilder(string target, string name, string author)
        {
            info = new ThemeFileManifest()
            {
                Version = CommonInfo.NxThemeFormatVersion,
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
}
