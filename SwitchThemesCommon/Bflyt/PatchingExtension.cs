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
		public static string[] GetPaneNames(this BflytFile f) =>
			f.Panes.Select(x => (x as INamedPane)?.PaneName).ToArray();

		public static bool ClearUVData(this BflytFile f, string name)
		{
			var PaneNames = f.GetPaneNames();
			int index = Array.IndexOf(PaneNames, name);
			if (index < 0) return false;

			var e = f.Panes[index] as Pic1Pane;
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
			string[] paneNames = f.GetPaneNames();
			for (int i = 0; i < Patches.Length; i++)
			{
				int index = Array.IndexOf(paneNames, Patches[i].PaneName);
				if (index == -1)
					return false;
				var p = Patches[i];
				var e = new Pan1Pane(f.Panes[index], f.FileByteOrder);
				f.Panes[index] = e;
				if (p.Visible != null)
					e.Visible = p.Visible.Value;
				#region ChangeTransform
				if (p.Position != null)
				{
					e.Position = new Vector3(
						p.Position.Value.X ?? e.Position.X,
						p.Position.Value.Y ?? e.Position.Y,
						p.Position.Value.Z ?? e.Position.Z);
				}
				if (p.Rotation != null)
				{
					e.Rotation = new Vector3(
						p.Rotation.Value.X ?? e.Rotation.X,
						p.Rotation.Value.Y ?? e.Rotation.Y,
						p.Rotation.Value.Z ?? e.Rotation.Z);
				}
				if (p.Scale != null)
				{
					e.Scale = new Vector2(
						p.Scale.Value.X ?? e.Scale.X,
						p.Scale.Value.Y ?? e.Scale.Y);
				}
				if (p.Size != null)
				{
					e.Size = new Vector2(
						p.Size.Value.X ?? e.Size.X,
						p.Size.Value.Y ?? e.Size.Y);
				}
				#endregion
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
				/*#region ColorDataForPic1
                if (e.name == "pic1")
                {
                    if (p.ColorTL != null)
                        e.ColorData[0] = Convert.ToUInt32(p.ColorTL, 16);
                    if (p.ColorTR != null)
                        e.ColorData[1] = Convert.ToUInt32(p.ColorTR, 16);
                    if (p.ColorBL != null)
                        e.ColorData[2] = Convert.ToUInt32(p.ColorBL, 16);
                    if (p.ColorBR != null)
                        e.ColorData[3] = Convert.ToUInt32(p.ColorBR, 16);
                }
                #endregion*/
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
					usd.ApplyChanges();
				}
				#endregion
			}
			return true;
		}

		public static bool ApplyMaterialsPatch(this BflytFile f, MaterialPatch[] Patches)
		{
			if (Patches == null) return true;
			var mats = f.GetMat;
			if (mats == null) return false;
			foreach (var p in Patches)
			{
				var target = mats.Materials.Where(x => x.Name == p.MaterialName).First();
				if (target == null) continue; //Less strict patching
				if (p.ForegroundColor != null)
					target.ForegroundColor = ByteStringLEToColor(p.ForegroundColor);
				if (p.BackgroundColor != null)
					target.BackgroundColor = ByteStringLEToColor(p.BackgroundColor);
			}
			return true;
		}

		static Color ByteStringLEToColor(string col)
		{
			uint Col = Convert.ToUInt32(col, 16);
			return new Color((byte)(Col & 0xFF), (byte)((Col >> 8) & 0xFF), (byte)((Col >> 16) & 0xFF), (byte)((Col >> 24) & 0xFF));
			//((uint)(col.R | col.G << 8 | col.B << 16 | col.A << 24))
		}

		public static bool AddGroupNames(this BflytFile f, ExtraGroup[] Groups)
		{
			if (Groups == null || Groups.Length == 0) return true;
			var PaneNames = f.GetPaneNames();
			var GroupNames = f.GetGroupNames();

			int rootGroupIndex = f.Panes.FindLastIndex(x => x.name == "gre1"); //find last group child list and append our groups there (aka at the end of RootGroup)
			if (rootGroupIndex == -1)
			{
				rootGroupIndex = f.Panes.FindIndex(x => x.name == "grp1");
				if (rootGroupIndex == -1)
					return false;
				f.Panes.Insert(rootGroupIndex + 1, new BasePane("gre1", 8));
				f.Panes.Insert(rootGroupIndex + 1, new BasePane("grs1", 8));
				rootGroupIndex += 2;
			}

			foreach (var g in Groups)
			{
				if (GroupNames.ContainsStr(g.GroupName)) continue;
				foreach (var s in g.Panes) if (!PaneNames.ContainsStr(s)) return false;
				f.Panes.Insert(rootGroupIndex, new Grp1Pane(f.version) { GroupName = g.GroupName, Panes = g.Panes.ToList() });
			}

			return true;
		}

		public static bool PatchTextureName(this BflytFile f, string original, string _new)
		{
			bool patchedSomething = false;
			var texSection = f.GetTex;
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

		static int AddBgMat(this BflytFile f, string TexName)
		{
			var MatSect = f.GetMat;
			#region AddTextures
			var texSection = f.GetTex;
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
					MatSect.Materials.Add(new BflytMaterial(mem.ToArray(), bin.ByteOrder, f.version));
				}
			}
			#endregion
			return MatSect.Materials.Count - 1;
		}

		static bool AddBgPanel(this BflytFile f, int index, string TexName, string Pic1Name)
		{
			#region add picture
			if (Pic1Name.Length > 0x18)
				throw new Exception("Pic1Name should not be longer than 24 chars");
			var BgPanel = new BasePane("pic1", 0x8);
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
				BgPanel.data = strm.ToArray();
			}
			#endregion
			f.Panes.Insert(index, new Pic1Pane(BgPanel, f.FileByteOrder));
			return true;
		}

		public static bool PatchBgLayout(this BflytFile f, PatchTemplate patch)
		{
			#region DetectPatch
			for (int i = 0; i < f.Panes.Count; i++)
			{
				var p = f.Panes[i] as Pic1Pane;
				if (p == null) continue;
				if (p.PaneName == patch.PatchIdentifier) return true;
				if (p.PaneName == "3x3lxBG") //Fix old layout
				{
					f.Panes.Remove(p);
					f.GetTex.Textures[0] = "White1x1^r";
					f.GetMat.Materials.RemoveAt(1);
				}
			}
			#endregion
			#region FindAndRemoveTargetBgPanels
			int target = int.MaxValue;
			for (int i = 0; i < f.Panes.Count - 1; i++)
			{
				string name = (f.Panes[i] as INamedPane)?.PaneName;
				if (name != null && patch.targetPanels.Contains(name))
				{
					if (i < target) target = i;
					if (patch.DirectPatchPane)
					{
						int m = f.AddBgMat(patch.MaintextureName);
						using (BinaryDataWriter bin = new BinaryDataWriter(new MemoryStream(f.Panes[i].data)))
						{
							bin.ByteOrder = ByteOrder.LittleEndian;
							bin.BaseStream.Position = 0x64 - 8;
							bin.Write((UInt16)m);
							f.Panes[i].data = ((MemoryStream)bin.BaseStream).ToArray();
						}
					}
					else if (!patch.NoRemovePanel)
					{
						using (BinaryDataWriter bin = new BinaryDataWriter(new MemoryStream(f.Panes[i].data)))
						{
							bin.ByteOrder = ByteOrder.LittleEndian;
							bin.BaseStream.Position = 0x24;
							bin.Write(5000f);
							bin.Write(60000f);
							f.Panes[i].data = ((MemoryStream)bin.BaseStream).ToArray();
						}
					}
				}
			}
			if (target == int.MaxValue) return false;
			#endregion
			if (!patch.DirectPatchPane)
				return f.AddBgPanel(target, patch.MaintextureName, patch.PatchIdentifier);
			else return true;
		}
	}
}
