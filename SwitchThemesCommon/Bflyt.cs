using ExtensionMethods;
using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ValueType = SwitchThemes.Common.Usd1Pane.EditableProperty.ValueType;

namespace SwitchThemes.Common
{
	public class BflytFile
	{
		public BasePanel this[int index]
		{
			get => Panels[index];
			set => Panels[index] = value;
		}

		public class BasePanel
		{
			public override string ToString()
			{
				return $"Panel {name} len: 0x{length.ToString("X")}";
			}

			public readonly string name;
			public Int32 length;
			public byte[] data;

			public BasePanel(string _name, int len)
			{
				name = _name;
				length = len;
				data = new byte[length - 8];
			}

			//used for PropertyEditablePanel, data is not cloned so it can be changed from the other classs
			public BasePanel(BasePanel basePane) 
			{
				name = basePane.name;
				length = basePane.length;
				data = basePane.data;
			}

			public BasePanel(string _name, BinaryDataReader bin)
			{
				name = _name;
				length = bin.ReadInt32();
				data = bin.ReadBytes(length - 8);
			}

			public virtual void WritePanel(BinaryDataWriter bin)
			{
				bin.Write(name, BinaryStringFormat.NoPrefixOrTermination);
				length = data.Length + 8;
				bin.Write(length);
				bin.Write(data);
			}
		}

		public class PropertyEditablePanel : BasePanel
		{
			public enum OriginX : byte
			{
				Center = 0,
				Left = 1,
				Right = 2
			};

			public enum OriginY : byte
			{
				Center = 0,
				Top = 1,
				Bottom = 2
			};

			public override string ToString()
			{
				return $"Panel {name} {PaneName}";
			}

			public readonly string PaneName;
			public Vector3 Position;
			public Vector3 Rotation;
			public Vector2 Scale;
			public Vector2 Size;

			byte _flag1;
			public bool Visible
			{
				get => (_flag1 & 0x1) == 0x1;
				set
				{
					if (value)
						_flag1 |= 0x1;
					else
						_flag1 &= 0xFE;
				}
			}

			byte _flag2;
			public OriginX originX
			{
				get => (OriginX)((_flag2 & 0xC0) >> 6);
				set
				{
					_flag2 &= unchecked((byte)(~0xC0));
					_flag2 |= (byte)((byte)value << 6);
				}
			}

			public OriginY originY
			{
				get => (OriginY)((_flag2 & 0x30) >> 4);
				set
				{
					_flag2 &= unchecked((byte)(~0x30));
					_flag2 |= (byte)((byte)value << 4);
				}
			}

			public OriginX ParentOriginX
			{
				get => (OriginX)((_flag2 & 0xC) >> 2);
				set
				{
					_flag2 &= unchecked((byte)(~0xC));
					_flag2 |= (byte)((byte)value << 2);
				}
			}

			public OriginY ParentOriginY
			{
				get => (OriginY)((_flag2 & 0x3));
				set
				{
					_flag2 &= unchecked((byte)(~0x3));
					_flag2 |= (byte)value;
				}
			}

			//only for pic1 panes
			public struct UVCoord
			{
				public Vector2 TopLeft;
				public Vector2 TopRight;
				public Vector2 BottomLeft;
				public Vector2 BottomRight;
			}

			public UVCoord[] UVCoords = null;
			public uint[] ColorData = null;

			public PropertyEditablePanel(BasePanel p) : base(p)
			{
				BinaryDataReader dataReader = new BinaryDataReader(new MemoryStream(data));
				dataReader.ByteOrder = ByteOrder.LittleEndian;
				_flag1 = dataReader.ReadByte();
				_flag2 = dataReader.ReadByte();
				dataReader.BaseStream.Position += 2;
				PaneName = dataReader.ReadFixedLenString(0x18);
				dataReader.BaseStream.Position = 0x2C - 8;
				Position = dataReader.ReadVector3();
				Rotation = dataReader.ReadVector3();
				Scale = dataReader.ReadVector2();
				Size = dataReader.ReadVector2();
				if (name == "pic1")
				{
					dataReader.BaseStream.Position = 0x54 - 8;
					ColorData = dataReader.ReadUInt32s(4);
					dataReader.ReadUInt16(); //material index
					UVCoords = new UVCoord[dataReader.ReadByte()];
					dataReader.ReadByte(); //padding
					for (int i = 0; i < UVCoords.Length; i++)
						UVCoords[i] = new UVCoord()
						{
							TopLeft = dataReader.ReadVector2(),
							TopRight = dataReader.ReadVector2(),
							BottomLeft = dataReader.ReadVector2(),
							BottomRight = dataReader.ReadVector2()
						};
				}
			}

			public void ApplyChanges()
			{
				using (var mem = new MemoryStream())
				{
					BinaryDataWriter bin = new BinaryDataWriter(mem);
					bin.ByteOrder = ByteOrder.LittleEndian;
					bin.Write(data);
					bin.BaseStream.Position = 0;
					bin.Write(_flag1);
					bin.Write(_flag2);
					bin.BaseStream.Position = 0x2C - 8;
					bin.Write(Position);
					bin.Write(Rotation);
					bin.Write(Scale);
					bin.Write(Size);
					if (name == "pic1")
					{
						bin.BaseStream.Position = 0x54 - 8;
						bin.Write(ColorData);
						bin.BaseStream.Position += 4;
						for (int i = 0; i < UVCoords.Length; i++)
						{
							bin.Write(UVCoords[i].TopLeft);
							bin.Write(UVCoords[i].TopRight);
							bin.Write(UVCoords[i].BottomLeft);
							bin.Write(UVCoords[i].BottomRight);
						}
					}
					data = mem.ToArray();
				}
			}

			public override void WritePanel(BinaryDataWriter bin)
			{
				ApplyChanges();
				base.WritePanel(bin);
			}
		}

		public class TextureSection : BasePanel
		{
			public List<string> Textures = new List<string>();
			public TextureSection(BinaryDataReader bin) : base("txl1", bin)
			{
				BinaryDataReader dataReader = new BinaryDataReader(new MemoryStream(data));
				dataReader.ByteOrder = bin.ByteOrder;
				int texCount = dataReader.ReadInt32();
				uint BaseOff = (uint)dataReader.Position;
				var Offsets = dataReader.ReadInt32s(texCount);
				foreach (var off in Offsets)
				{
					dataReader.Position = BaseOff + off;
					Textures.Add(dataReader.ReadString(BinaryStringFormat.ZeroTerminated));
				}
			}

			public TextureSection() : base("txl1", 8) { }

			public override void WritePanel(BinaryDataWriter bin)
			{
				var newData = new MemoryStream();
				BinaryDataWriter dataWriter = new BinaryDataWriter(newData);
				dataWriter.ByteOrder = bin.ByteOrder;
				dataWriter.Write(Textures.Count);
				dataWriter.Write(new int[Textures.Count]);
				for (int i = 0; i < Textures.Count; i++)
				{
					uint off = (uint)dataWriter.Position;
					dataWriter.Write(Textures[i], BinaryStringFormat.ZeroTerminated);
					while (dataWriter.BaseStream.Position % 4 != 0)
						dataWriter.Write((byte)0);
					uint endPos = (uint)dataWriter.Position;
					dataWriter.Position = 4 + i * 4;
					dataWriter.Write(off - 4);
					dataWriter.Position = endPos;
				}
				data = newData.ToArray();
				base.WritePanel(bin);
			}
		}

		public class MaterialsSection : BasePanel
		{
			public uint Version;

			public List<BflytPanes.BflytMaterial> Materials = new List<BflytPanes.BflytMaterial>();
			public MaterialsSection(BinaryDataReader bin, uint ver) : base("mat1", bin)
			{
				Version = ver;
				BinaryDataReader dataReader = new BinaryDataReader(new MemoryStream(data));
				dataReader.ByteOrder = bin.ByteOrder;
				int matCount = dataReader.ReadInt32();
				var Offsets = dataReader.ReadInt32s(matCount).Select(x => x - 8).ToArray(); // offsets relative to the stream
				for (int i = 0; i < matCount; i++)
				{
					int matLen = (i == matCount - 1 ? (int)dataReader.BaseStream.Length : Offsets[i + 1]) - (int)dataReader.Position;
					Materials.Add(new BflytPanes.BflytMaterial(dataReader.ReadBytes(matLen), dataReader.ByteOrder, ver));
				}
			}

			public MaterialsSection() : base("mat1", 8) { }

			public override void WritePanel(BinaryDataWriter bin)
			{
				var newData = new MemoryStream();
				BinaryDataWriter dataWriter = new BinaryDataWriter(newData);
				dataWriter.ByteOrder = bin.ByteOrder;
				dataWriter.Write(Materials.Count);
				dataWriter.Write(new int[Materials.Count]);
				for (int i = 0; i < Materials.Count; i++)
				{
					uint off = (uint)dataWriter.Position;
					dataWriter.Write(Materials[i].Write(Version, dataWriter.ByteOrder));
					uint endPos = (uint)dataWriter.Position;
					dataWriter.Position = 4 + i * 4;
					dataWriter.Write(off + 8);
					dataWriter.Position = endPos;
				}
				data = newData.ToArray();
				base.WritePanel(bin);
			}
		}

		public class PicturePanel : BasePanel
		{
			public readonly string PanelName;
			public PicturePanel(BinaryDataReader bin) : base("pic1", bin)
			{
				PanelName = TryGetPanelName(this);
			}

			public override string ToString()
			{
				return $"Picture {PanelName}";
			}
		}

		public static string TryGetPanelName(BasePanel p)
		{
			if (p.data.Length < 0x18 + 4) return null;
			BinaryDataReader dataReader = new BinaryDataReader(new MemoryStream(p.data), Encoding.ASCII, false);
			dataReader.ByteOrder = ByteOrder.LittleEndian;
			dataReader.ReadInt32(); //Unknown
			string PanelName = "";
			for (int i = 0; i < 0x18; i++)
			{
				var c = dataReader.ReadChar();
				if (c == 0) break;
				PanelName += c;
			}
			return PanelName;
		}

		public List<BasePanel> Panels = new List<BasePanel>();

		UInt32 version;

		public byte[] SaveFile()
		{
			var res = new MemoryStream();
			BinaryDataWriter bin = new BinaryDataWriter(res);
			bin.ByteOrder = ByteOrder.LittleEndian;
			bin.Write("FLYT", BinaryStringFormat.NoPrefixOrTermination);
			bin.Write((byte)0xFF);
			bin.Write((byte)0xFE); //Little endian
			bin.Write((UInt16)0x14); //Header size
			bin.Write(version);
			bin.Write((Int32)0);
			bin.Write((UInt16)Panels.Count);
			bin.Write((UInt16)0); //padding
			foreach (var p in Panels)
				p.WritePanel(bin);
			while (bin.BaseStream.Position % 4 != 0)
				bin.Write((byte)0);
			bin.BaseStream.Position = 0xC;
			bin.Write((uint)bin.BaseStream.Length);
			bin.BaseStream.Position = bin.BaseStream.Length;
			return res.ToArray();
		}

		public enum PatchResult
		{
			AlreadyPatched,
			Fail,
			CorruptedFile,
			OK
		}

		public bool PatchTextureName(string original, string _new)
		{
			bool patchedSomething = false;
			var texSection = GetTex;
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

		int AddBgMat(string TexName)
		{
			var MatSect = GetMat;
			#region AddTextures
			var texSection = GetTex;
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
					MatSect.Materials.Add(new BflytPanes.BflytMaterial(mem.ToArray(), bin.ByteOrder, version));
				}
			}
			#endregion
			return MatSect.Materials.Count - 1;
		}

		PatchResult AddBgPanel(int index, string TexName, string Pic1Name)
		{
			#region add picture
			if (Pic1Name.Length > 0x18)
				throw new Exception("Pic1Name should not be longer than 24 chars");
			var BgPanel = new BasePanel("pic1", 0x8);
			Panels.Insert(index, BgPanel);
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
				bin.Write((UInt16)AddBgMat(TexName));
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
			
			return PatchResult.OK;
		}

		public string[] GetPaneNames()
		{
			string[] paneNames = new string[Panels.Count];
			for (int i = 0; i < Panels.Count; i++)
				paneNames[i] = TryGetPanelName(Panels[i]);
			return paneNames;
		}
		
		public string[] GetGroupNames() => Panels.Where(x => x is Grp1Pane).Select(x => ((Grp1Pane) x).GroupName).ToArray();

		public PatchResult ApplyLayoutPatch(PanePatch[] Patches)
		{
			string[] paneNames = GetPaneNames();
			for (int i = 0; i < Patches.Length; i++)
			{
				int index = Array.IndexOf(paneNames, Patches[i].PaneName);
				if (index == -1)
					//return PatchResult.CorruptedFile;
					continue;
					//The layout patching has been made less strict to allow some 8.x layouts to work on lower firmwares

				var p = Patches[i];
				var e = new PropertyEditablePanel(Panels[index]);
				Panels[index] = e;
				if (p.Visible != null)
					e.Visible = p.Visible.Value;
				#region ChangeTransform
				if (p.Position != null)
				{
					e.Position.X = p.Position.Value.X ?? e.Position.X;
					e.Position.Y = p.Position.Value.Y ?? e.Position.Y;
					e.Position.Z = p.Position.Value.Z ?? e.Position.Z;
				}
				if (p.Rotation != null)
				{
					e.Rotation.X = p.Rotation.Value.X ?? e.Rotation.X;
					e.Rotation.Y = p.Rotation.Value.Y ?? e.Rotation.Y;
					e.Rotation.Z = p.Rotation.Value.Z ?? e.Rotation.Z;
				}
				if (p.Scale != null)
				{
					e.Scale.X = p.Scale.Value.X ?? e.Scale.X;
					e.Scale.Y = p.Scale.Value.Y ?? e.Scale.Y;
				}
				if (p.Size != null)
				{
					e.Size.X = p.Size.Value.X ?? e.Size.X;
					e.Size.Y = p.Size.Value.Y ?? e.Size.Y;
				}
				#endregion
				#region Change other prperties
				if (p.OriginX != null)
					e.originX = (PropertyEditablePanel.OriginX)p.OriginX.Value;
				if (p.OriginY != null)
					e.originY = (PropertyEditablePanel.OriginY)p.OriginY.Value;
				if (p.ParentOriginX != null)
					e.ParentOriginX = (PropertyEditablePanel.OriginX)p.ParentOriginX.Value;
				if (p.ParentOriginY != null)
					e.ParentOriginY = (PropertyEditablePanel.OriginY)p.ParentOriginY.Value;
				#endregion
				#region ColorDataForPic1
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
				#endregion
				#region usdPane
				if (p.UsdPatches != null && Panels.Count > index + 1 && Panels[index + 1].name == "usd1")
				{
					Usd1Pane usd = (Usd1Pane)Panels[index + 1];
					foreach (var patch in p.UsdPatches)
					{
						var v = usd.FindName(patch.PropName);
						if (v == null)
							usd.AddNewProperty(patch.PropName, patch.PropValues, (ValueType)patch.type);
						if (v != null && v.ValueCount == patch.PropValues.Length && (int)v.type == patch.type)
							v.value = patch.PropValues;
					}
				}
				#endregion
			}
			return PatchResult.OK;
		}

		public PatchResult ApplyMaterialsPatch(MaterialPatch[] Patches)
		{
			if (Patches == null) return PatchResult.OK;
			var mats = GetMat;
			if (mats == null) return PatchResult.Fail;
			foreach (var p in Patches)
			{
				var target = mats.Materials.Where(x => x.Name == p.MaterialName).FirstOrDefault();
				if (target == null) continue; //Less strict patching
				if (p.ForegroundColor != null)
					target.ForegroundColor = Convert.ToUInt32(p.ForegroundColor, 16);
				if (p.BackgroundColor != null)
					target.BackgroundColor = Convert.ToUInt32(p.BackgroundColor, 16);
			}
			return PatchResult.OK;
		}

		public PatchResult AddGroupNames(ExtraGroup[] Groups)
		{
			if (Groups == null || Groups.Length == 0) return PatchResult.OK;
			var PaneNames = GetPaneNames();
			var GroupNames = GetGroupNames();

			int rootGroupIndex = Panels.FindLastIndex(x => x.name == "gre1"); //find last group child list and append our groups there (aka at the end of RootGroup)
			if (rootGroupIndex == -1)
			{
				rootGroupIndex = Panels.FindIndex(x => x.name == "grp1");
				if (rootGroupIndex == -1)
					return PatchResult.CorruptedFile;
				Panels.Insert(rootGroupIndex + 1, new BasePanel("gre1", 8));
				Panels.Insert(rootGroupIndex + 1, new BasePanel("grs1", 8));
				rootGroupIndex += 2;
			}

			foreach (var g in Groups)
			{
				if (GroupNames.ContainsStr(g.GroupName)) continue;
				foreach (var s in g.Panes) if (!PaneNames.ContainsStr(s))
						return PatchResult.Fail;
				Panels.Insert(rootGroupIndex, new Grp1Pane(version) { GroupName = g.GroupName, Panes = g.Panes.ToList() });
			}

			return PatchResult.OK;
		}

		public PatchResult PatchBgLayout(PatchTemplate patch)
		{
			#region DetectPatch
			for (int i = 0; i < Panels.Count; i++)
			{
				if (!(Panels[i] is PicturePanel)) continue;
				var p = Panels[i] as PicturePanel;
				if (p.PanelName == patch.PatchIdentifier) return PatchResult.AlreadyPatched;
				if (p.PanelName == "3x3lxBG") //Fix old layout
				{
					Panels.Remove(p);
					GetTex.Textures[0] = "White1x1^r";
					GetMat.Materials.RemoveAt(1);
				}
			}
			#endregion
			#region FindAndRemoveTargetBgPanels
			int target = int.MaxValue;
			for (int i = 0; i < Panels.Count - 1; i++)
			{
				string name = TryGetPanelName(Panels[i]);
				if (name != null && patch.targetPanels.Contains(name))
				{
					if (i < target) target = i;
					if (patch.DirectPatchPane)
					{
						int m = AddBgMat(patch.MaintextureName);
						using (BinaryDataWriter bin = new BinaryDataWriter(new MemoryStream(Panels[i].data)))
						{
							bin.ByteOrder = ByteOrder.LittleEndian;
							bin.BaseStream.Position = 0x64 - 8;
							bin.Write((UInt16)m);
							Panels[i].data = ((MemoryStream)bin.BaseStream).ToArray();
						}
					}
					else if (!patch.NoRemovePanel)
					{
						using (BinaryDataWriter bin = new BinaryDataWriter(new MemoryStream(Panels[i].data)))
						{
							bin.ByteOrder = ByteOrder.LittleEndian;
							bin.BaseStream.Position = 0x24;
							bin.Write(5000f);
							bin.Write(60000f);
							Panels[i].data = ((MemoryStream)bin.BaseStream).ToArray();
						}
					}
				}
			}
			if (target == int.MaxValue) return PatchResult.Fail;
			#endregion
			if (!patch.DirectPatchPane)
				return AddBgPanel(target, patch.MaintextureName, patch.PatchIdentifier);
			else return PatchResult.OK;
		}

		public PatchResult ClearUVData(string name)
		{
			var PaneNames = GetPaneNames();
			int index = Array.IndexOf(PaneNames, name);
			if (index < 0 || Panels[index].name != "pic1") return PatchResult.Fail;

			var e = new PropertyEditablePanel(Panels[index]);
			Panels[index] = e;
			for (int i = 0; i < e.UVCoords.Length; i++)
			{
				e.UVCoords[i].TopLeft = new Vector2(0, 0);
				e.UVCoords[i].TopRight = new Vector2(1, 0);
				e.UVCoords[i].BottomLeft = new Vector2(0, 1);
				e.UVCoords[i].BottomRight = new Vector2(1, 1);
			}

			return PatchResult.OK;
		}

		public TextureSection GetTex
		{
			get
			{
				var res = (TextureSection)Panels.Find(x => x is TextureSection); ;
				if (res == null)
				{
					res = new TextureSection();
					Panels.Insert(2, res);
				}
				return res;
			}
		}

		public MaterialsSection GetMat
		{
			get
			{
				var res = (MaterialsSection)Panels.Find(x => x is MaterialsSection);
				if (res == null)
				{
					res = new MaterialsSection();
					Panels.Insert(3, res);
				}
				return res;
			}
		}

		public BflytFile(byte[] data) : this(new MemoryStream(data)) { }

		public BflytFile(Stream file)
		{
			BinaryDataReader bin = new BinaryDataReader(file);
			bin.ByteOrder = ByteOrder.LittleEndian;
			if (bin.ReadString(4) != "FLYT") throw new Exception("Wrong signature");
			bin.ReadUInt16(); //BOM
			bin.ReadUInt16(); //HeaderSize
			version = bin.ReadUInt32();
			bin.ReadUInt32(); //File size
			var sectionCount = bin.ReadUInt16();
			bin.ReadUInt16(); //padding
			for (int i = 0; i < sectionCount; i++)
			{
				string name = bin.ReadString(4);
				switch (name)
				{
					case "txl1":
						Panels.Add(new TextureSection(bin));
						break;
					case "mat1":
						Panels.Add(new MaterialsSection(bin, version));
						break;
					case "pic1":
						Panels.Add(new PicturePanel(bin));
						break;
					case "usd1":
						Panels.Add(new Usd1Pane(bin));
						break;
					case "grp1":
						Panels.Add(new Grp1Pane(bin, version));
						break;
					default:
						Panels.Add(new BasePanel(name, bin));
						break;
				}
				if (i == sectionCount - 1 && bin.BaseStream.Position != bin.BaseStream.Length) //load sections missing in the section count (my old bflyt patch)
				{
					while (bin.PeekChar() == 0 && bin.BaseStream.Position < bin.BaseStream.Length) bin.ReadChar();
					if (bin.BaseStream.Length - bin.BaseStream.Position >= 8) //min section size
					{
						sectionCount++;
					}
				}
			}
		}
	}
}
