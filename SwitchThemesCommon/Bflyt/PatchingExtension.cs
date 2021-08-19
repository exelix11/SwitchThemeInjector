
using ExtensionMethods;
using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using static SwitchThemes.Common.Bflyt.BflytFile;

namespace SwitchThemes.Common.Bflyt
{
	public static class BflytExten
	{
		public static bool ClearUVData(this BflytFile f, string name)
		{
			var e = f[name] as Pic1Pane;
			if (e == null) return false;
			for (int i = 0; i < e.UVCoords.Length; i++)
			{
				e.UVCoords[i].TopLeft = new Vector2(0, 0);
				e.UVCoords[i].TopRight = new Vector2(1, 0);
				e.UVCoords[i].BottomLeft = new Vector2(0, 1);
				e.UVCoords[i].BottomRight = new Vector2(1, 1);
			}

			return true;
		}

		public static bool ApplyLayoutPatch(this BflytFile f, PanePatch[] Patches)
		{
			for (int i = 0; i < Patches.Length; i++)
			{
				var p = Patches[i];
				var e = f[Patches[i].PaneName] as Pan1Pane;
				
				// The layout patching has been made less strict to allow some 8.x layouts to work on lower firmwares, not sure if this is a good idea in the layout editor as a way to detect layout incompatibilities may be desiderable.
				if (e == null)
					continue;


				e.Visible = p.Visible ?? e.Visible;
				
				e.Position = p.Position ?? e.Position;
				e.Rotation = p.Rotation ?? e.Rotation;
				e.Scale = p.Scale ?? e.Scale;
				e.Size = p.Size ?? e.Size;

				#region Change other prperties
				if (p.OriginX != null)
					e.originX = (Pan1Pane.OriginX)p.OriginX.Value;
				if (p.OriginY != null)
					e.originY = (Pan1Pane.OriginY)p.OriginY.Value;
				if (p.ParentOriginX != null)
					e.ParentOriginX = (Pan1Pane.OriginX)p.ParentOriginX.Value;
				if (p.ParentOriginY != null)
					e.ParentOriginY = (Pan1Pane.OriginY)p.ParentOriginY.Value;
				#endregion
				#region ColorDataForPic1
                if (e.name == "pic1")
                {
					var ee = e as Pic1Pane;
                    if (p.PaneSpecific0 != null)
                        ee.ColorTopLeft = new RGBAColor(p.PaneSpecific0);
                    if (p.PaneSpecific1 != null)
                        ee.ColorTopRight = new RGBAColor(p.PaneSpecific1);
                    if (p.PaneSpecific2 != null)
                        ee.ColorBottomLeft = new RGBAColor(p.PaneSpecific2);
                    if (p.PaneSpecific3 != null)
                        ee.ColorBottomRight = new RGBAColor(p.PaneSpecific3);
				}
				#endregion
				#region ColorForTextPanes
				if (e.name == "txt1")
				{
					var ee = e as Txt1Pane;
					if (p.PaneSpecific0 != null)
						ee.FontTopColor = new RGBAColor(p.PaneSpecific0);
					if (p.PaneSpecific1 != null)
						ee.ShadowTopColor = new RGBAColor(p.PaneSpecific1);
					if (p.PaneSpecific2 != null)
						ee.FontBottomColor = new RGBAColor(p.PaneSpecific2);
					if (p.PaneSpecific3 != null)
						ee.ShadowBottomColor = new RGBAColor(p.PaneSpecific3);
				}
				#endregion
				#region usdPane
				if (e.UserData != null && p.UsdPatches != null)
				{
					Usd1Pane usd = e.UserData;
					foreach (var patch in p.UsdPatches)
					{
						var v = usd.FindName(patch.PropName);
						if (v == null)
							usd.Properties.Add(new Usd1Pane.EditableProperty() { Name = patch.PropName, value = patch.PropValues, type = (Usd1Pane.EditableProperty.ValueType)patch.type });
						if (v != null && v.ValueCount == patch.PropValues.Length && (int)v.type == patch.type)
							v.value = patch.PropValues;
					}
					//usd.ApplyChanges();
				}
				#endregion
			}
			return true;
		}

		public static bool ApplyMaterialsPatch(this BflytFile f, MaterialPatch[] Patches)
		{
			if (Patches == null) return true;
			var mats = f.Mat1;
			if (mats == null) return false;
			foreach (var p in Patches)
			{
				foreach (var target in mats.Materials.Where(x => x.Name == p.MaterialName))
				{
					if (p.ForegroundColor != null)
						target.ForegroundColor = new RGBAColor(p.ForegroundColor);
					if (p.BackgroundColor != null)
						target.BackgroundColor = new RGBAColor(p.BackgroundColor);

					if (p.Refs == null && p.Transforms == null)
						continue;

					Dictionary<string, int> texToMadId = new Dictionary<string, int>();
					for (int i = 0; i < target.Textures.Length; i++)
					{
						var id = target.Textures[i].TextureId;
						texToMadId.Add(f.Tex1.Textures[id], i);
					}

					foreach (var rp in p.Refs)
					{
						if (!texToMadId.ContainsKey(rp.Name))
							continue;

						var tex = target.Textures[texToMadId[rp.Name]];

						if (rp.WrapS != null)
							tex.WrapS = (BflytMaterial.TextureReference.WRAPS)rp.WrapS.Value;

						if (rp.WrapT != null)
							tex.WrapT = (BflytMaterial.TextureReference.WRAPS)rp.WrapT.Value;
					}

					foreach (var tp in p.Transforms)
					{
						if (!texToMadId.ContainsKey(tp.Name))
							continue;

						var tf = target.TextureTransformations[texToMadId[tp.Name]];

						tf.Rotation = tp.Rotation ?? tf.Rotation;
						tf.ScaleX = tp.ScaleX ?? tf.ScaleX;
						tf.ScaleY = tp.ScaleY ?? tf.ScaleY;
						tf.X = tp.X ?? tf.X;
						tf.Y = tp.Y ?? tf.Y;
					}
				}			
			}
			return true;
		}

		public static bool AddGroupNames(this BflytFile f, ExtraGroup[] Groups)
		{
			if (Groups == null || Groups.Length == 0) return true;
			if (f.RootGroup == null) return false;

			var PanesWithNames = f.EnumeratePanes().Where(x => x is INamedPane).Cast<INamedPane>();
			var GroupNames = PanesWithNames.Where(x => x is Grp1Pane).Select(x => ((Grp1Pane)x).name).ToArray();
			var PaneNames = PanesWithNames.Where(x => !(x is Grp1Pane)).Select(x => x.PaneName);

			foreach (var g in Groups)
			{
				if (GroupNames.Contains(g.GroupName)) continue;
				foreach (var s in g.Panes) if (!PaneNames.Contains(s)) return false;
				f.RootGroup.Children.Add(new Grp1Pane(f.Version) { GroupName = g.GroupName, Panes = g.Panes.ToList() });
			}

			return true;
		}

		public static bool PatchTextureName(this BflytFile f, string original, string _new)
		{
			bool patchedSomething = false;
			var texSection = f.Tex1;
			if (texSection == null) return false;
			for (int i = 0; i < texSection.Textures.Count; i++)
			{
				if (texSection.Textures[i] == original)
				{
					patchedSomething = true;
					texSection.Textures[i] = _new;
				}
			}
			return patchedSomething;
		}

		static ushort AddBgMat(this BflytFile f, string TexName)
		{
			var MatSect = f.GetMaterialsSection();
			#region AddTextures
			var texSection = f.GetTexturesSection();
			if (!texSection.Textures.Contains(TexName))
				texSection.Textures.Add(TexName);
			int texIndex = texSection.Textures.IndexOf(TexName);
			#endregion
			#region Add material
			{
				MemoryStream mem = new MemoryStream();
				using (BinaryDataWriter bin = new BinaryDataWriter(mem))
				{
					bin.ByteOrder = ByteOrder.LittleEndian;
					bin.Write("P_Custm", BinaryStringFormat.ZeroTerminated);
					bin.Write(new byte[0x14]);
					bin.Write((Int32)0x15);
					bin.Write((Int32)0x8040200);
					bin.Write((Int32)0);
					bin.Write((UInt32)0xFFFFFFFF);
					bin.Write((UInt16)texIndex);
					bin.Write((UInt16)0x0);
					bin.Write(new byte[0xC]);
					bin.Write(1f);
					bin.Write(1f);
					bin.Write(new byte[0x10]);
					MatSect.Materials.Add(new BflytMaterial(mem.ToArray(), bin.ByteOrder, f.Version));
				}
			}
			#endregion
			return (ushort)(MatSect.Materials.Count - 1);
		}

		static private bool AddBgPanel(this BflytFile f, BasePane target, string TexName, string Pic1Name)
		{
			#region add picture
			if (Pic1Name.Length > 0x18)
				throw new Exception("Pic1Name should not be longer than 24 chars");
			var strm = new MemoryStream();
			using (BinaryDataWriter bin = new BinaryDataWriter(strm))
			{
				bin.ByteOrder = ByteOrder.LittleEndian;
				bin.Write((byte)0x01);
				bin.Write((byte)0x00);
				bin.Write((byte)0xFF);
				bin.Write((byte)0x04);
				bin.Write(Pic1Name, BinaryStringFormat.NoPrefixOrTermination);
				int zerCount = Pic1Name.Length;
				while (zerCount++ < 0x38)
					bin.Write((byte)0x00);
				bin.Write(1f);
				bin.Write(1f);
				bin.Write(1280f);
				bin.Write(720f);
				bin.Write((UInt32)0xFFFFFFFF);
				bin.Write((UInt32)0xFFFFFFFF);
				bin.Write((UInt32)0xFFFFFFFF);
				bin.Write((UInt32)0xFFFFFFFF);
				bin.Write((UInt16)f.AddBgMat(TexName));
				bin.Write((UInt16)1);
				bin.Write((UInt32)0);
				bin.Write((UInt32)0);
				bin.Write(1f);
				bin.Write((UInt32)0);
				bin.Write((UInt32)0);
				bin.Write(1f);
				bin.Write(1f);
				bin.Write(1f);
			}
			#endregion
			BasePane p = new BasePane("pic1", 8);
			p.data = strm.ToArray(); 
			target.Parent.Children.Insert(target.Parent.Children.IndexOf(target),p);
			return true;
		}

		public static bool PatchBgLayout(this BflytFile f, PatchTemplate patch)
		{
			#region DetectPatch
			if (f[patch.PatchIdentifier] != null) return true;
			{
				var p = f["3x3lxBG"];
				if (p != null)
				{
					f.RemovePane(p);
					f.Tex1.Textures[0] = "White1x1^r";
					f.Mat1.Materials.RemoveAt(1);
				}
			}
			#endregion
			#region FindAndRemoveTargetBgPanels
			BasePane target = null;
			foreach (var t in patch.targetPanels)
			{
				var p = f[t];
				if (p == null) continue;
				if (target == null) target = p;
				if (patch.DirectPatchPane)
				{
					ushort m = f.AddBgMat(patch.MaintextureName);
					var pe = p as Pic1Pane;
					pe.MaterialIndex = m;
				}
				else if (!patch.NoRemovePanel)
				{
					var pe = p as Pan1Pane;
					pe.Position = new Vector3(5000, 60000, 0);
				}
			}
			if (target == null) return false;
			#endregion
			if (!patch.DirectPatchPane)
				return f.AddBgPanel(target, patch.MaintextureName, patch.PatchIdentifier);
			else return true;
		}

		public static bool PanePullToFront(this BflytFile f, string paneName)
		{
			var target = f[paneName];
			if (target == null) return false;
			f.MovePane(target, target.Parent, 0);
			return true;
		}

		public static bool PanePushBack(this BflytFile f, string paneName)
		{
			var target = f[paneName];
			if (target == null) return false;
			f.MovePane(target, target.Parent, target.Parent.Children.Count);
			return true;
		}
	}
}
