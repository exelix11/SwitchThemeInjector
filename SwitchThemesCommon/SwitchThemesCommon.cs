using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.Linq;
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

        public PatchPartInfo(string name, string szsName, string description, 
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

        public const string PartHome = "home";
        public const string PartLock = "lock";
        public const string PartApps = "apps";
        public const string PartSet = "set";
        public const string PartNews = "news";
        public const string PartQlaunchCommon = "qlaunch_common";
        public const string PartUser = "user";
        public const string PartPlayerSelect = "psl";

        public static List<PatchPartInfo> Parts = new List<PatchPartInfo>()
        {
            new PatchPartInfo(PartHome, "ResidentMenu.szs", "Home menu"),
            new PatchPartInfo(PartLock, "Entrance.szs", "Lock screen"),
            new PatchPartInfo(PartApps, "Flaunch.szs", "Player select"),
            new PatchPartInfo(PartSet , "Set.szs", "All apps menu"),
            new PatchPartInfo(PartNews, "Notification.szs", "Settings applet"),
            new PatchPartInfo(PartQlaunchCommon, "commmon.szs", "Common layout properties", allowImages: false),
            new PatchPartInfo(PartUser, "MyPage.szs", "User page", "0100000000001013"),
            new PatchPartInfo(PartPlayerSelect, "Psl.szs" , "News applet", "0100000000001007"),
        };

        public static PatchPartInfo GetPart(string name) =>
            Parts.Where(x => x.Name == name).FirstOrDefault();
    }
}
