using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using ExtensionMethods;
using SARCExt;
using SwitchThemes.Common;
using SwitchThemes.Common.Bflan;
using SwitchThemes.Common.Bflyt;
using SwitchThemes.Common.Serializers;
using Syroot.BinaryData;
using static SwitchThemes.Common.Bflyt.BflytFile;

namespace SwitchThemes.Common
{
	public static class LayoutDiff
	{
		readonly static string[] IgnorePaneList = new string[] { "usd1", "lyt1", "mat1", "txl1", "fnl1", "grp1", "pae1", "pas1", "cnt1" };

		public static LayoutPatch Diff(SarcData original, SarcData edited)
		{
			List<LayoutFilePatch> Patches = new List<LayoutFilePatch>();
			if (!ScrambledEquals<string>(original.Files.Keys, edited.Files.Keys))
			{
				MessageBox.Show("The provided archives don't have the same files");
				return null;
			}

			bool hasAtLeastAnExtraGroup = false; //Used to detect if animations are properly implemented
			foreach (var f in original.Files.Keys.Where(x => x.EndsWith(".bflyt")))
			{
				if (original.Files[f].SequenceEqual(edited.Files[f])) continue;
				BflytFile or = new BflytFile(original.Files[f]);
				BflytFile ed = new BflytFile(edited.Files[f]);
				string[] orPaneNames = GetPaneNames(or);
				string[] edPaneNames = GetPaneNames(ed);
				List<PanePatch> curFile = new List<PanePatch>();
				for (int i = 0; i < edPaneNames.Length; i++)
				{
					if (edPaneNames[i] == null || !(ed[i] is Pan1Pane) || IgnorePaneList.Contains(ed[i].name)) continue;
					var j = Array.IndexOf(orPaneNames, edPaneNames[i]);
					if (j == -1) continue;

					PanePatch curPatch = new PanePatch() { PaneName = edPaneNames[i] };
					curPatch.UsdPatches = MakeUsdPatch(or[j].UserData, ed[i].UserData);
					if (ed[i].data.SequenceEqual(or[j].data))
					{
						if (curPatch.UsdPatches == null) continue;
						curFile.Add(curPatch);
						continue;
					}

					var orPan = (Pan1Pane)(or[j]);
					var edPan = (Pan1Pane)(ed[i]);
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

					if (edPan.originX != orPan.originX)
						curPatch.OriginX = (byte)edPan.originX;
					if (edPan.originY != orPan.originY)
						curPatch.OriginY = (byte)edPan.originY;
					if (edPan.ParentOriginX != orPan.ParentOriginX)
						curPatch.ParentOriginX = (byte)edPan.ParentOriginX;
					if (edPan.ParentOriginY != orPan.ParentOriginY)
						curPatch.ParentOriginY = (byte)edPan.ParentOriginY;

					if (edPan is Pic1Pane && orPan is Pic1Pane)
					{
						var edPic = (Pic1Pane)edPan;
						var orPic = (Pic1Pane)orPan;
						if (edPic.ColorTopLeft != orPic.ColorTopLeft)
							curPatch.ColorTL = edPic.ColorTopLeft.AsHexLEString();
						if (edPic.ColorTopRight != orPic.ColorTopRight)
							curPatch.ColorTR = edPic.ColorTopRight.AsHexLEString();
						if (edPic.ColorBottomLeft != orPic.ColorBottomLeft)
							curPatch.ColorBL = edPic.ColorBottomLeft.AsHexLEString();
						if (edPic.ColorBottomRight != orPic.ColorBottomRight)
							curPatch.ColorBR = edPic.ColorBottomRight.AsHexLEString();
					}
					curFile.Add(curPatch);
				}

				List<ExtraGroup> extraGroups = new List<ExtraGroup>();
				string[] ogPanes = or.GetGroupNames();
				foreach (var p_ in ed.Panes.Where(x => x is Grp1Pane))
				{
					var p = ((Grp1Pane)p_);
					if (ogPanes.Contains(p.GroupName)) continue;
					extraGroups.Add(new ExtraGroup() { GroupName = p.GroupName, Panes = p.Panes.ToArray() });
					hasAtLeastAnExtraGroup = true;
				}
				if (extraGroups.Count == 0) extraGroups = null;

				List<MaterialPatch> materials = new List<MaterialPatch>();
				if (ed.GetMat != null && or.GetMat != null)
				{					
					var edMat = ed.GetMat;
					foreach (var orM in or.GetMat.Materials)
					{
						var edM = edMat.Materials.Where(x => x.Name == orM.Name).FirstOrDefault();
						if (edM == null) continue;
						if (edM.ForegroundColor == orM.ForegroundColor && edM.BackgroundColor == orM.BackgroundColor) continue;
						MaterialPatch m = new MaterialPatch() { MaterialName = orM.Name };
						if (edM.ForegroundColor != orM.ForegroundColor)
							m.ForegroundColor = edM.ForegroundColor.AsHexLEString();
						if (edM.BackgroundColor != orM.BackgroundColor)
							m.BackgroundColor = edM.BackgroundColor.AsHexLEString();
						materials.Add(m);
					}
				}
				if (materials.Count == 0) materials = null;

				if (curFile.Count > 0 || extraGroups?.Count > 0 || materials?.Count > 0)
					Patches.Add(new LayoutFilePatch() { FileName = f, Patches = curFile.ToArray(), Materials = materials?.ToArray(), AddGroups = extraGroups?.ToArray() });
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
				BflanFile anim = new BflanFile(edited.Files[f]);
				AnimPatches.Add(new AnimFilePatch() { FileName = f, AnimJson = BflanSerializer.ToJson(anim) });
			}
			if (AnimPatches.Count == 0) AnimPatches = null;
			else if (!hasAtLeastAnExtraGroup) MessageBox.Show("This theme uses custom animations but doesn't have custom group in the layouts, this means that the nxtheme will work on the firmware it has been developed on but it may break on older or newer ones. It's *highly recommended* to create custom groups to handle animations");

			return new LayoutPatch()
			{
				//TODO: PatchName = "diffPatch" + (targetPatch == null ? "" : " for " + targetPatch.TemplateName),
				PatchName = "diffPatch",
				AuthorName = "autoDiff",
				Files = Patches.ToArray(),
				Anims = AnimPatches?.ToArray(),
				Ready8X = true
			};
		}

		static List<UsdPatch> MakeUsdPatch(Usd1Pane or, Usd1Pane ed)
		{
			if (or == null || ed == null) return null;
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

		static string[] GetPaneNames(BflytFile layout)
		{
			string TryGetPaneName(BasePane p)
			{
				if (p.data.Length < 0x18 + 4) return null;
				BinaryDataReader dataReader = new BinaryDataReader(new MemoryStream(p.data), Encoding.ASCII, false);
				dataReader.ByteOrder = layout.FileByteOrder;
				dataReader.ReadInt32(); //Unknown
				string PaneName = "";
				for (int i = 0; i < 0x18; i++)
				{
					var c = dataReader.ReadChar();
					if (c == 0) break;
					PaneName += c;
				}
				return PaneName;
			}

			List<string> str = new List<string>();
			foreach (var p in layout.Panes)
			{
				string res = null;
				if (!IgnorePaneList.Contains(p.name))
					res = TryGetPaneName(p);
				str.Add(res);
			}
			return str.ToArray();
		}

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
