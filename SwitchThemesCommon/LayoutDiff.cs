using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
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

		public struct DiffOptions 
		{
			public bool? HideOnlineButton;
		}

		public static (LayoutPatch,string) Diff(SarcData original, SarcData edited, DiffOptions? opt)
		{
			List<LayoutFilePatch> Patches = new List<LayoutFilePatch>();
			if (!ScrambledEquals<string>(original.Files.Keys, edited.Files.Keys))
				throw new Exception("The provided archives don't have the same files");

			bool hasAtLeastAnExtraGroup = false; //Used to detect if animations are properly implemented
			foreach (var f in original.Files.Keys.Where(x => x.EndsWith(".bflyt")))
			{
				if (original.Files[f].SequenceEqual(edited.Files[f])) continue;
				BflytFile _or = new BflytFile(original.Files[f]);
				BflytFile _ed = new BflytFile(edited.Files[f]);

				var curFile = DiffPanes(_or, _ed, f);

				var extraGroups = DiffGroups(_or, _ed);
				if (extraGroups != null)
					hasAtLeastAnExtraGroup = true;

				var materials = DiffMaterials(_or, _ed, f);

				if (curFile.Count > 0 || extraGroups?.Count > 0 || materials?.Count > 0)
					Patches.Add(new LayoutFilePatch() { FileName = f, Patches = curFile.ToArray(), Materials = materials?.ToArray(), AddGroups = extraGroups?.ToArray() });
			}

			string Message = null;

			List<AnimFilePatch> AnimPatches = new List<AnimFilePatch>();
			foreach (var f in original.Files.Keys.Where(x => x.EndsWith(".bflan")))
			{
				if (original.Files[f].SequenceEqual(edited.Files[f])) continue;
				BflanFile anim = new BflanFile(edited.Files[f]);
				AnimPatches.Add(new AnimFilePatch() { FileName = f, AnimJson = BflanSerializer.ToJson(anim) });
			}
			if (AnimPatches.Count == 0) AnimPatches = null;
			else if (!hasAtLeastAnExtraGroup)
				Message = "This theme uses custom animations but doesn't have custom group in the layouts, this means that the nxtheme will work on the firmware it has been developed on but it may break on older or newer ones. It's *highly recommended* to create custom groups to handle animations";

			if (AnimPatches != null && AnimPatches.Any(x => x.FileName == "anim/RdtBase_SystemAppletPos.bflan"))
			{
				if (opt == null || opt?.HideOnlineButton == null)
					opt = new DiffOptions { HideOnlineButton = false };
				else if (opt != null && opt.Value.HideOnlineButton.Value)
				{
					Message = "You chose to hide the 11.0+ \"Switch online\" button but manually edited the \"RdtBase_SystemAppletPos\" animation. HideOnlineButton will be disabled.";
					opt = new DiffOptions { HideOnlineButton = false };
				}
			}
				
			var targetPatch = DefaultTemplates.GetFor(original);
			return (new LayoutPatch()
			{
				PatchName = "diffPatch" + (targetPatch == null ? "" : " for " + targetPatch.TemplateName),
				TargetName = targetPatch?.szsName,
				AuthorName = "autoDiff",
				Files = Patches.ToArray(),
				Anims = AnimPatches?.ToArray(),
				ID = $"Generated_{Guid.NewGuid()}",
				HideOnlineBtn = targetPatch?.NXThemeName != "home" ? null : opt?.HideOnlineButton
			}, Message);
		}

		static List<MaterialPatch> DiffMaterials(BflytFile _or, BflytFile _ed, string layoutname)
		{
			(BflytMaterial.TextureReference, BflytMaterial.TextureTransofrm)? OriginalTex(string name, BflytMaterial m)
			{
				var id = _or.Tex1.Textures.IndexOf(name);
				if (id == -1) 
					return null;

				if (!m.Textures.Any(x => x.TextureId == id))
					return null;

				var tx = m.Textures.First(x => x.TextureId == id);
				var tr = m.TextureTransformations[Array.IndexOf(m.Textures, tx)];

				return (tx, tr);
			}

			List<MaterialPatch> materials = new List<MaterialPatch>();
			if (_ed.Mat1 != null && _or.Mat1 != null)
			{
				var edMat = _ed.Mat1;
				foreach (var orM in _or.Mat1.Materials)
				{
					var edM = edMat.Materials.Where(x => x.Name == orM.Name).FirstOrDefault();
					if (edM == null) continue;

					MaterialPatch m = new MaterialPatch();

					if (edM.ForegroundColor != orM.ForegroundColor)
						m.ForegroundColor = edM.ForegroundColor.AsHexLEString();

					if (edM.BackgroundColor != orM.BackgroundColor)
						m.BackgroundColor = edM.BackgroundColor.AsHexLEString();

					var transforms = new List<MaterialPatch.TexTransform>();
					var refs = new List<MaterialPatch.TexReference>();
					for (int i = 0; i < edM.Textures.Length; i++)
					{
						var tx = edM.Textures[i];
						var tf = edM.TextureTransformations[i];
						var tex = _ed.Tex1.Textures[tx.TextureId];

						var original = OriginalTex(tex, orM);

						if (original != null)
						{
							var (ortx, ortf) = original.Value;

							if (ortf != tf)
								transforms.Add(new MaterialPatch.TexTransform()
								{
									Name = tex,
									Rotation = ortf.Rotation == tf.Rotation ? (float?)null : tf.Rotation,
									ScaleX = ortf.ScaleX == tf.ScaleX ? (float?)null : tf.ScaleX,
									ScaleY = ortf.ScaleY == tf.ScaleY ? (float?)null : tf.ScaleY,
									X = ortf.X == tf.X ? (float?)null : tf.X,
									Y = ortf.Y == tf.Y ? (float?)null : tf.Y
								}); ;

							if (ortx != tx)
								refs.Add(new MaterialPatch.TexReference()
								{
									Name = tex,
									WrapS = (byte)tx.WrapS,
									WrapT = (byte)tx.WrapT
								});
						}
						/*
							At some point they started adding multiple materials with the same name but different texture references, this breaks
							the following assumption:
						
							else throw new Exception($"A texture reference to {tex} in the edited layout is not part of the original file: {layoutname}");

							For now i'm leaving diffing commented so diffing doesn't crash but materials are probably to be considered broken until this is rewritten.
							Guess an option could be matching materials by index.
						 */
					}

					m.Transforms = transforms.Count == 0 ? null : transforms.ToArray();
					m.Refs = refs.Count == 0 ? null : refs.ToArray();

					if (m.IsEmpty())
						continue;

					m.MaterialName = orM.Name;
					materials.Add(m);
				}
			}
			
			if (materials.Count == 0) 
				materials = null;

			return materials;
		}

		static List<ExtraGroup> DiffGroups(BflytFile _or, BflytFile _ed)
		{
			List<ExtraGroup> extraGroups = new List<ExtraGroup>();
			
			string[] ogPanes = _or.EnumeratePanes(_or.RootGroup).Select(x => ((Grp1Pane)x).GroupName).ToArray();
			var edPanes = _ed.EnumeratePanes(_ed.RootGroup).Cast<Grp1Pane>();
			foreach (var p in edPanes)
			{
				if (ogPanes.Contains(p.GroupName)) continue;
				extraGroups.Add(new ExtraGroup() { GroupName = p.GroupName, Panes = p.Panes.ToArray() });
			}
			
			if (extraGroups.Count == 0) 
				extraGroups = null;

			return extraGroups;
		}

		static List<PanePatch> DiffPanes(BflytFile _or, BflytFile _ed, string filename)
		{
			List<PanePatch> curFile = new List<PanePatch>();
			foreach (var orpane_ in _or.EnumeratePanes().Where(x => x is INamedPane))
			{
				var edpane = _ed[((INamedPane)orpane_).PaneName];
				if (edpane == null) throw new Exception($"{filename} is missing {((INamedPane)orpane_).PaneName}");
				if (orpane_.name != edpane.name) throw new Exception($"{filename} : {((INamedPane)orpane_).PaneName} Two panes with the same name are of a different type");
				if (IgnorePaneList.Contains(orpane_.name)) continue;

				var edPan = (Pan1Pane)edpane;
				var orPan = (Pan1Pane)orpane_;

				PanePatch curPatch = new PanePatch() { PaneName = edPan.PaneName };
				curPatch.UsdPatches = MakeUsdPatch(edPan.UserData, orPan.UserData);
				if (edPan.data.SequenceEqual(orPan.data))
				{
					if (curPatch.UsdPatches != null)
						curFile.Add(curPatch);
					continue;
				}

				if (edPan.Position != orPan.Position)
					curPatch.Position = edPan.Position;
				if (edPan.Rotation != orPan.Rotation)
					curPatch.Rotation = edPan.Rotation;
				if (edPan.Scale != orPan.Scale)
					curPatch.Scale = edPan.Scale;
				if (edPan.Size != orPan.Size)
					curPatch.Size = edPan.Size;
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
						curPatch.PaneSpecific0 = edPic.ColorTopLeft.AsHexLEString();
					if (edPic.ColorTopRight != orPic.ColorTopRight)
						curPatch.PaneSpecific1 = edPic.ColorTopRight.AsHexLEString();
					if (edPic.ColorBottomLeft != orPic.ColorBottomLeft)
						curPatch.PaneSpecific2 = edPic.ColorBottomLeft.AsHexLEString();
					if (edPic.ColorBottomRight != orPic.ColorBottomRight)
						curPatch.PaneSpecific3 = edPic.ColorBottomRight.AsHexLEString();
				}

				if (edPan is Txt1Pane && orPan is Txt1Pane)
				{
					var edTxt = (Txt1Pane)edPan;
					var orTxt = (Txt1Pane)orPan;
					if (edTxt.FontTopColor != orTxt.FontTopColor)
						curPatch.PaneSpecific0 = edTxt.FontTopColor.AsHexLEString();
					if (edTxt.ShadowTopColor != orTxt.ShadowTopColor)
						curPatch.PaneSpecific1 = edTxt.ShadowTopColor.AsHexLEString();
					if (edTxt.FontBottomColor != orTxt.FontBottomColor)
						curPatch.PaneSpecific2 = edTxt.FontBottomColor.AsHexLEString();
					if (edTxt.ShadowBottomColor != orTxt.ShadowBottomColor)
						curPatch.PaneSpecific3 = edTxt.ShadowBottomColor.AsHexLEString();
				}
				curFile.Add(curPatch);
			}

			return curFile;
		}

		static List<UsdPatch> MakeUsdPatch(Usd1Pane ed, Usd1Pane or)
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
