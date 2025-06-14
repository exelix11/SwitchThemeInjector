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

        public static List<CompatIssue> ValidateLayout(SarcData szs, LayoutPatch layout)
        {
            var res = new List<CompatIssue>();

            // First do layouts, none of these are critical since we can ignore missing panes
            foreach (var p in layout.Files)
            {
                if (!szs.Files.ContainsKey(p.FileName))
                {
                    res.Add(CompatIssue.MissingFile(p.FileName));
                    continue;
                }

                var bflyt = new BflytFile(szs.Files[p.FileName]);
                var paneNames = bflyt.EnumeratePanes().Select(x => x.name).ToHashSet();

                if (p.Patches != null)
                    foreach (var pane in p.Patches)
                        if (!paneNames.Contains(pane.PaneName))
                            res.Add(CompatIssue.MissingPane(p.FileName, pane.PaneName));

                if (p.AddGroups != null)
                    foreach (var group in p.AddGroups)
                        foreach (var item in group.Panes)
                            if (!paneNames.Contains(item))
                                res.Add(CompatIssue.MissingPane(p.FileName, item, $"group:{group.GroupName}"));

                if (p.PullFrontPanes != null)
                    foreach (var pane in p.PullFrontPanes)
                        if (!paneNames.Contains(pane))
                            res.Add(CompatIssue.MissingPane(p.FileName, pane, $"PullFrontPanes"));

                if (p.PushBackPanes != null)
                    foreach (var pane in p.PushBackPanes)
                        if (!paneNames.Contains(pane))
                            res.Add(CompatIssue.MissingPane(p.FileName, pane, $"PullFrontPanes"));
            
                // TODO: Materials
            }

            // Then do animations
            foreach (var anim in layout.Anims)
            {
                if (!szs.Files.ContainsKey(anim.FileName))
                {
                    res.Add(CompatIssue.MissingFile(anim.FileName));
                    continue;
                }

                var bflytName = anim.FileName.Split('/').Last().Split('_').First();
                bflytName = "blyt/" + bflytName + ".bflyt";

                if (!szs.Files.ContainsKey(bflytName))
                {
                    res.Add(CompatIssue.Uncertain(anim.FileName, bflytName, "Unknown bflyt file"));
                    continue;
                }

                var bflyt = new BflytFile(szs.Files[bflytName]);
                var paneNames = bflyt.EnumeratePanes().Select(x => x.name).ToHashSet();
                var groupNames = bflyt.EnumeratePanes().Where(x => x is Grp1Pane).Select(x => x.name).ToHashSet();
                var layoutPatch = layout.Files.FirstOrDefault(x => x.FileName == bflytName);

                var bflan = BflanSerializer.FromJson(anim.AnimJson);

                foreach (var group in bflan.patData.Groups)
                {
                    if (groupNames.Contains(group))
                        continue;

                    // The group might also be added via a patch
                    if (layoutPatch != null && layoutPatch.AddGroups != null)
                    {
                        if (layoutPatch.AddGroups.Any(x => x.GroupName == group))
                            continue;
                    }

                    res.Add(CompatIssue.MissingGroup(anim.FileName, group));
                }

                foreach (var entry in bflan.paiData.Entries)
                {
                    if (entry.Target == Pai1Section.PaiEntry.AnimationTarget.Pane)
                        if (!paneNames.Contains(entry.Name))
                            res.Add(CompatIssue.MissingPane(anim.FileName, entry.Name, "Animation target", true));

                    // TODO: Materials
                    // TODO: Textures
                }
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