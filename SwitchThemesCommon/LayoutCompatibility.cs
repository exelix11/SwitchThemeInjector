using SARCExt;
using SwitchThemes.Common.Bflan;
using SwitchThemes.Common.Bflyt;
using SwitchThemes.Common.Serializers;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SwitchThemes.Common
{
    public class LayoutCompatibility
    {
        public enum ProblemType
        {
            // The layout references a file that does not exist anymore
            MissingFile,
            // The layout or animation references a pane that does not exist anymore
            MissingPane,
            // The animation references a group that does not exist anymore
            MissingGroup,
            // The animation references a texture that does not exist anymore
            MissingTexture,
            // The heuristic failed to determine the severity of the issue
            Uncertain
        }

        public enum ProblemSeverity
        {
            // This issue won't crash the theme, it will be automatically ignored by the theme installer
            // it is the case for layout patches that refer to non-existing panes
            AutoIgnored,
            // This issue will crash the console if the theme is installed
            // It is the case for custom animations that refer to non-existing panes or groups
            // To prevent crashes during the installation process any file that has a Critical issue will be dropped
            Critical
        }

        public class CompatIssue
        {
            public string FileName;
            public string ItemName;
            public string AdditionalInfo;
            public ProblemType Type;
            public ProblemSeverity Severity;

            public static CompatIssue MissingFile(string fileName) => new CompatIssue
            {
                FileName = fileName,
                ItemName = fileName,
                AdditionalInfo = "",
                Type = ProblemType.MissingFile,
                Severity = ProblemSeverity.AutoIgnored
            };

            public static CompatIssue MissingPane(string fileName, string paneName, string additional = "", bool critical = false) => new CompatIssue
            {
                FileName = fileName,
                ItemName = paneName,
                AdditionalInfo = additional,
                Type = ProblemType.MissingPane,
                Severity = critical ? ProblemSeverity.Critical : ProblemSeverity.AutoIgnored
            };

            public static CompatIssue Uncertain(string fileName, string itemName, string additional) => new CompatIssue
            {
                FileName = fileName,
                ItemName = itemName,
                AdditionalInfo = additional,
                Type = ProblemType.Uncertain,
                Severity = ProblemSeverity.AutoIgnored
            };

            public static CompatIssue MissingGroup(string fileName, string groupName) => new CompatIssue
            {
                FileName = fileName,
                ItemName = groupName,
                AdditionalInfo = "",
                Type = ProblemType.MissingGroup,
                Severity = ProblemSeverity.Critical
            };
        }

        public static string LayoutNameForAnimation(string animation)
        {
            // The animation file name is usually in the format "anim/filename_animname.bflan"
            // We want to extract the "filename" part
            var parts = animation.Split('/');
            if (parts.Length < 2)
                return null;

            var fileName = parts[1].Split('_').FirstOrDefault();
            if (string.IsNullOrEmpty(fileName))
                return null;

            return "blyt/" + fileName + ".bflyt";
        }

        public static void CheckBflytCompatibility(List<CompatIssue> res, LayoutFilePatch patch, BflytFile bflyt) 
        {
            var paneNames = bflyt.EnumeratePanes()
                .Where(x => x is BflytFile.INamedPane)
                .Select(x => ((BflytFile.INamedPane)x).PaneName)
                .ToHashSet();

            if (patch.Patches != null)
                foreach (var pane in patch.Patches)
                    if (!paneNames.Contains(pane.PaneName))
                        res.Add(CompatIssue.MissingPane(patch.FileName, pane.PaneName));

            if (patch.AddGroups != null)
                foreach (var group in patch.AddGroups)
                    foreach (var item in group.Panes)
                        if (!paneNames.Contains(item))
                            res.Add(CompatIssue.MissingPane(patch.FileName, item, $"group:{group.GroupName}"));

            if (patch.PullFrontPanes != null)
                foreach (var pane in patch.PullFrontPanes)
                    if (!paneNames.Contains(pane))
                        res.Add(CompatIssue.MissingPane(patch.FileName, pane, $"PullFrontPanes"));

            if (patch.PushBackPanes != null)
                foreach (var pane in patch.PushBackPanes)
                    if (!paneNames.Contains(pane))
                        res.Add(CompatIssue.MissingPane(patch.FileName, pane, $"PullFrontPanes"));

            // TODO: Materials
        }

        public static void CheckAnimationCompatibility(List<CompatIssue> res, LayoutPatch layout, SarcData szs, string animName, BflanFile bflan) 
        {
            var bflytName = LayoutNameForAnimation(animName);

            if (!szs.Files.ContainsKey(bflytName))
            {
                res.Add(CompatIssue.Uncertain(animName, bflytName, "Unknown bflyt file"));
                return;
            }

            var bflyt = new BflytFile(szs.Files[bflytName]);
            var paneNames = bflyt.EnumeratePanes()
                .Where(x => x is BflytFile.INamedPane)
                .Select(x => ((BflytFile.INamedPane)x).PaneName)
                .ToHashSet();

            var groupNames = bflyt.EnumeratePanes()
                .Where(x => x is Grp1Pane)
                .Select(x => ((Grp1Pane)x).PaneName)
                .ToHashSet();

            var layoutPatch = layout.Files.Where(x => x.FileName == bflytName).ToArray();

            // Target groups might also be added via a patch
            var addedGroups = layoutPatch
                .Where(x => x.AddGroups != null).SelectMany(x => x.AddGroups)
                .Select(x => x.GroupName)
                .ToHashSet();

            foreach (var group in bflan.patData.Groups)
            {
                if (groupNames.Contains(group))
                    continue;

                if (addedGroups.Contains(group))
                    continue;

                res.Add(CompatIssue.MissingGroup(animName, group));
            }

            foreach (var entry in bflan.paiData.Entries)
            {
                if (entry.Target == Pai1Section.PaiEntry.AnimationTarget.Pane)
                    if (!paneNames.Contains(entry.Name))
                        res.Add(CompatIssue.MissingPane(animName, entry.Name, "Animation target", true));

                // TODO: Materials
                // TODO: Textures
            }
        }

        public static List<CompatIssue> ValidateLayout(SarcData szs, LayoutPatch layout)
        {
            var res = new List<CompatIssue>();

            // First do layouts, none of these are critical since we can ignore missing panes
            foreach (var patch in layout.Files)
            {
                if (!szs.Files.ContainsKey(patch.FileName))
                {
                    res.Add(CompatIssue.MissingFile(patch.FileName));
                    continue;
                }

                var bflyt = new BflytFile(szs.Files[patch.FileName]);
                CheckBflytCompatibility(res, patch, bflyt);
            }

            // Then do animations
            foreach (var anim in layout.Anims)
            {
                if (!szs.Files.ContainsKey(anim.FileName))
                {
                    res.Add(CompatIssue.MissingFile(anim.FileName));
                    continue;
                }

                var bflan = BflanSerializer.FromJson(anim.AnimJson);
                CheckAnimationCompatibility(res, layout, szs, anim.FileName, bflan);
            }

            return res;
        }
  
        public static string StringifyIssues(List<CompatIssue> issues)
        {
            if (issues.Count == 0)
                return "No compatibility issues found.";

            StringBuilder sb = new StringBuilder();

            foreach (var issue in issues.GroupBy(x => x.FileName))
            {
                sb.AppendLine($"File: {issue.Key}");
                foreach (var item in issue)
                {
                    sb.AppendLine($"   - Item: {item.ItemName}");
                    sb.AppendLine($"     Type: {item.Type}");
                    sb.AppendLine($"     Severity: {item.Severity}");
                    if (!string.IsNullOrEmpty(item.AdditionalInfo))
                        sb.AppendLine($"     Additional Info: {item.AdditionalInfo}");
                    sb.AppendLine();
                }
            }

            return sb.ToString();
        }
    }
}