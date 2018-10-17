using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using SARCExt;
using SwitchThemes.Common;

namespace SwitchThemes
{
	public static class LayoutDiff
	{
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
					if (ed[i].data.Length < 0x4C) continue;
					if (f == skipLayoutName && targetPatch.targetPanels.Contains(edPaneNames[i])) continue;
					var j = Array.IndexOf(orPaneNames, edPaneNames[i]);
					if (j == -1) continue;
					if (ed[i].data.SequenceEqual(or[j].data)) continue;
					PanePatch curPatch = new PanePatch() { PaneName = edPaneNames[i] };
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
				if (curFile.Count > 0)
					Patches.Add(new LayoutFilePatch() { FileName = f, Patches = curFile.ToArray() });
			}
			if (Patches.Count == 0)
			{
				MessageBox.Show("Couldn't find any difference");
				return null; 
			}
			return new LayoutPatch()
			{
				PatchName = "diffPatch" + targetPatch == null ? "" : "for " + targetPatch.TemplateName,
				AuthorName = "autoDiff",
				Files = Patches.ToArray()
			};
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
