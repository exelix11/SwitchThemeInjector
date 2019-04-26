using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using SARCExt;
using SwitchThemes.Common;
using SwitchThemes.Common.Serializers;

namespace SwitchThemes
{
	public static class LayoutDiff
	{
		//Note: usd1 is ignored here as it's usually linked to the pane directly above it
		readonly static string[] IgnorePaneList = new string[] { "usd1", "lyt1", "mat1", "txl1", "fnl1", "grp1", "pae1", "pas1", "cnt1" };

		public static LayoutPatch Diff(SarcData original, SarcData edited)
		{
			List<LayoutFilePatch> Patches = new List<LayoutFilePatch>();
			if (!ScrambledEquals<string>(original.Files.Keys, edited.Files.Keys))
			{
				MessageBox.Show("The provided archives don't have the same files");
				return null;
			}
			var targetPatch = SwitchThemesCommon.DetectSarc(original, DefaultTemplates.templates);
			string skipLayoutName = targetPatch != null ? targetPatch.MainLayoutName : "";

			bool hasAtLeastAnExtraGroup = false; //Used to detect if animations are properly implemented
			foreach (var f in original.Files.Keys.Where(x => x.EndsWith(".bflyt")))
			{
				if (original.Files[f].SequenceEqual(edited.Files[f])) continue;
				BflytFile or = new BflytFile(original.Files[f]);
				BflytFile ed = new BflytFile(edited.Files[f]);
				string[] orPaneNames = or.GetPaneNames();
				string[] edPaneNames = ed.GetPaneNames();
				List<PanePatch> curFile = new List<PanePatch>();
				for (int i = 0; i < edPaneNames.Length; i++)
				{
					if (ed[i].data.Length < 0x4C || IgnorePaneList.Contains(ed[i].name)) continue;
					if (f == skipLayoutName && (targetPatch?.targetPanels?.Contains(edPaneNames[i]) ?? false)) continue;
					var j = Array.IndexOf(orPaneNames, edPaneNames[i]);
					if (j == -1) continue;

					PanePatch curPatch = new PanePatch() { PaneName = edPaneNames[i] };

					curPatch.UsdPatches = MakeUsdPatch(or, i, ed, j);
					if (ed[i].data.SequenceEqual(or[j].data))
					{
						if (curPatch.UsdPatches == null) continue;
						curFile.Add(curPatch);
						continue;
					}

					var orPan = new BflytFile.PropertyEditablePanel(or[j]);
					var edPan = new BflytFile.PropertyEditablePanel(ed[i]);
					if (!VecEqual(edPan.Position, orPan.Position))
						curPatch.Position = ToNullVec(edPan.Position);
					if (!VecEqual(edPan.Rotation, orPan.Rotation))
						curPatch.Rotation = ToNullVec(edPan.Rotation);
					if (!VecEqual(edPan.Scale, orPan.Scale))
						curPatch.Scale = ToNullVec(edPan.Scale);
					if (!VecEqual(edPan.Size, orPan.Size))
						curPatch.Size = ToNullVec(edPan.Size);
					if (edPan.Visible != orPan.Visible)
						curPatch.Visible = edPan.Visible;
					if (edPan.name == "pic1")
					{
						if (edPan.ColorData[0] != orPan.ColorData[0])
							curPatch.ColorTL = edPan.ColorData[0].ToString("X");
						if (edPan.ColorData[1] != orPan.ColorData[1])
							curPatch.ColorTR = edPan.ColorData[1].ToString("X");
						if (edPan.ColorData[2] != orPan.ColorData[2])
							curPatch.ColorBL = edPan.ColorData[2].ToString("X");
						if (edPan.ColorData[3] != orPan.ColorData[3])
							curPatch.ColorBR = edPan.ColorData[3].ToString("X");
					}
					curFile.Add(curPatch);
				}

				List<ExtraGroup> extraGroups = new List<ExtraGroup>();
				string[] ogPanes = or.GetGroupNames();
				foreach (var p_ in ed.Panels.Where(x => x is Grp1Pane))
				{
					var p = ((Grp1Pane)p_);
					if (ogPanes.Contains(p.GroupName)) continue;
					extraGroups.Add(new ExtraGroup() { GroupName = p.GroupName, Panes = p.Panes.ToArray() });
					hasAtLeastAnExtraGroup = true;
				}
				if (extraGroups.Count == 0) extraGroups = null;

				if (curFile.Count > 0 || extraGroups?.Count > 0)
					Patches.Add(new LayoutFilePatch() { FileName = f, Patches = curFile.ToArray(), AddGroups = extraGroups?.ToArray() });
			}
			if (Patches.Count == 0) //animation edits depend on bflyt changes so this is relevant
			{
				MessageBox.Show("Couldn't find any difference");
				return null; 
			}

			List<AnimFilePatch> AnimPatches = new List<AnimFilePatch>();
			foreach (var f in original.Files.Keys.Where(x => x.EndsWith(".bflan")))
			{
				if (original.Files[f].SequenceEqual(edited.Files[f])) continue;
				Bflan anim = new Bflan(edited.Files[f]);
				AnimPatches.Add(new AnimFilePatch() { FileName = f, AnimJson = BflanSerializer.ToJson(anim) });
			}
			if (AnimPatches.Count == 0) AnimPatches = null;
			else if (!hasAtLeastAnExtraGroup) MessageBox.Show("This theme uses custom animations but doesn't have custom group in the layouts, this means that the nxtheme will work on the firmware it has been developed on but it may break on older or newer ones. It's *highly recommended* to create custom groups to handle animations");

			return new LayoutPatch()
			{
				PatchName = "diffPatch" + (targetPatch == null ? "" : "for " + targetPatch.TemplateName),
				AuthorName = "autoDiff",
				Files = Patches.ToArray(),
				Anims = AnimPatches?.ToArray(),
				Ready8X = true //Aka tell the patcher to not fix this layout
			};
		}

		static List<UsdPatch> MakeUsdPatch(BflytFile original, int oindex, BflytFile edited, int eindex)
		{
			if (original.Panels.Count <= oindex + 1) return null;
			if (edited.Panels.Count <= eindex + 1) return null;
			if (original.Panels[oindex + 1].name != "usd1" || edited.Panels[eindex + 1].name != "usd1") return null;

			Usd1Pane or = (Usd1Pane)original.Panels[oindex + 1];
			Usd1Pane ed = (Usd1Pane)edited.Panels[eindex + 1];
			if (or.data.SequenceEqual(ed.data)) return null;

			List<UsdPatch> res = new List<UsdPatch>();
			foreach (var edP in ed.Properties)
			{
				var orP = or.FindName(edP.Name);
				if (orP != null)
				{
					if (orP.ValueCount != edP.ValueCount) continue;
					if (orP.type != edP.type) continue;
					if (orP.type != Usd1Pane.EditableProperty.ValueType.int32 && orP.type != Usd1Pane.EditableProperty.ValueType.single) continue;
					
					if (orP.value.SequenceEqual(edP.value)) continue;
				}
				res.Add(new UsdPatch()
				{
					PropName = edP.Name,
					PropValues = edP.value,
					type = (int)edP.type
				});
			}

			if (res.Count == 0) return null;
			return res;
		}

		static bool VecEqual(Vector3 v, Vector3 v1) => v.X == v1.X && v.Y == v1.Y && v.Z == v1.Z;
		static NullableVector3 ToNullVec(Vector3 v) => new NullableVector3() { X = v.X, Y = v.Y, Z = v.Z };
		static bool VecEqual(Vector2 v, Vector2 v1) => v.X == v1.X && v.Y == v1.Y;
		static NullableVector2 ToNullVec(Vector2 v) => new NullableVector2() { X = v.X, Y = v.Y };

		public static bool ScrambledEquals<T>(IEnumerable<T> list1, IEnumerable<T> list2)
		{
			var cnt = new Dictionary<T, int>();
			foreach (T s in list1)
			{
				if (cnt.ContainsKey(s))
				{
					cnt[s]++;
				}
				else
				{
					cnt.Add(s, 1);
				}
			}
			foreach (T s in list2)
			{
				if (cnt.ContainsKey(s))
				{
					cnt[s]--;
				}
				else
				{
					return false;
				}
			}
			return cnt.Values.All(c => c == 0);
		}
	}
}
