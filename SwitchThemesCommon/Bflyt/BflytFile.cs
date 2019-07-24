using ExtensionMethods;
using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SwitchThemes.Common
{
	public interface IInspectable
	{
		byte[] GetData();
	}

	public class Color
	{
		public byte R, G, B, A;
		public Color(byte r, byte g, byte b, byte a = 255)
		{
			R = r; G = g; B = b; A = a;
		}

		public string AsHexLEString() =>
			((uint)(R | G << 8 | B << 16 | A << 24)).ToString("X8");
	}
}

namespace SwitchThemes.Common.Bflyt
{
	public class BflytFile
	{
		public BasePane this[int index]
		{
			get => Panes[index];
			set => Panes[index] = value;
		}

		public interface INamedPane
		{
			string PaneName { get; }
		}

		public ByteOrder FileByteOrder;
		public class BasePane : IInspectable
		{
			public BasePane Parent;
			public List<BasePane> Children = new List<BasePane>();

			[Browsable(true)]
			[TypeConverter(typeof(ExpandableObjectConverter))]
			virtual public Usd1Pane UserData { get; set; } = null;

			public override string ToString()
			{
				return $"[Unknown pane type: {name}]";
			}

			public readonly string name;
			public Int32 length;
			public byte[] data;

			public byte[] GetData() => data;

			public BasePane(string _name, int len)
			{
				name = _name;
				length = len;
				data = new byte[length - 8];
			}

			//used for PropertyEditablePane, data is not cloned so it can be changed from the other classs
			public BasePane(BasePane basePane)
			{
				name = basePane.name;
				length = basePane.length;
				data = basePane.data;
				if (name != "usd1")
					UserData = basePane.UserData;
			}

			public BasePane(string _name, BinaryDataReader bin)
			{
				name = _name;
				length = bin.ReadInt32();
				data = bin.ReadBytes(length - 8);
			}

			public BasePane(string _name, byte[] _data)
			{
				name = _name;
				data = _data;
				length = data.Length + 8;
			}

			public virtual void WritePane(BinaryDataWriter bin)
			{
				bin.Write(name, BinaryStringFormat.NoPrefixOrTermination);
				length = data.Length + 8;
				bin.Write(length);
				bin.Write(data);
				if (UserData != null)
					UserData.WritePane(bin);
			}

			public virtual BasePane Clone()
			{
				MemoryStream mem = new MemoryStream();
				BinaryDataWriter bin = new BinaryDataWriter(mem);
				WritePane(bin);
				BasePane res = new BasePane(name, (byte[])data.Clone());
				if (name != "usd1" && UserData != null)
					res.UserData = (Usd1Pane)UserData.Clone();
				return res;
			}

			protected virtual void ApplyChanges(BinaryDataWriter bin) { }
		}

		public class CusRectangle
		{
			public int x, y, width, height, scaleX, scaleY;

			public CusRectangle(int _x, int _y, int _width, int _height)
			{
				x = _x;
				y = _y;
				width = _width;
				height = _height;
			}
		}

		public class TextureSection : BasePane
		{
			public List<string> Textures = new List<string>();
			public TextureSection(BinaryDataReader bin) : base("txl1", bin)
			{
				BinaryDataReader dataReader = new BinaryDataReader(new MemoryStream(data));
				dataReader.ByteOrder = bin.ByteOrder;
				int texCount = dataReader.ReadInt16();
				dataReader.ReadInt16(); //padding
				uint BaseOff = (uint)dataReader.Position;
				var Offsets = dataReader.ReadInt32s(texCount);
				foreach (var off in Offsets)
				{
					dataReader.Position = BaseOff + off;
					Textures.Add(dataReader.ReadString(BinaryStringFormat.ZeroTerminated));
				}
			}

			public TextureSection() : base("txl1", 8) { }

			public override void WritePane(BinaryDataWriter bin)
			{
				var newData = new MemoryStream();
				BinaryDataWriter dataWriter = new BinaryDataWriter(newData);
				dataWriter.ByteOrder = bin.ByteOrder;
				dataWriter.Write((Int16)Textures.Count);
				dataWriter.Write((Int16)0); //padding
				dataWriter.Write(new int[Textures.Count]);
				for (int i = 0; i < Textures.Count; i++)
				{
					uint off = (uint)dataWriter.Position;
					dataWriter.Write(Textures[i], BinaryStringFormat.ZeroTerminated);
					uint endPos = (uint)dataWriter.Position;
					dataWriter.Position = 4 + i * 4;
					dataWriter.Write(off - 4);
					dataWriter.Position = endPos;
				}
				while (dataWriter.BaseStream.Position % 4 != 0)
					dataWriter.Write((byte)0);
				data = newData.ToArray();
				base.WritePane(bin);
			}
		}

		public class MaterialsSection : BasePane
		{
			public List<BflytMaterial> Materials = new List<BflytMaterial>();
			public uint version;

			public MaterialsSection(BinaryDataReader bin, uint ver) : base("mat1", bin)
			{
				version = ver;
				BinaryDataReader dataReader = new BinaryDataReader(new MemoryStream(data));
				dataReader.ByteOrder = bin.ByteOrder;
				int matCount = dataReader.ReadInt16();
				dataReader.ReadInt16(); //padding
				var Offsets = dataReader.ReadInt32s(matCount).Select(x => x - 8).ToArray(); // offsets relative to the stream
				for (int i = 0; i < matCount; i++)
				{
					int matLen = (i == matCount - 1 ? (int)dataReader.BaseStream.Length : Offsets[i + 1]) - (int)dataReader.Position;
					Materials.Add(new BflytMaterial(dataReader.ReadBytes(matLen), dataReader.ByteOrder, version));
				}
			}

			public MaterialsSection() : base("mat1", 8) { }

			public override void WritePane(BinaryDataWriter bin)
			{
				var newData = new MemoryStream();
				BinaryDataWriter dataWriter = new BinaryDataWriter(newData);
				dataWriter.ByteOrder = bin.ByteOrder;
				dataWriter.Write((Int16)Materials.Count);
				dataWriter.Write((Int16)0); //padding
				dataWriter.Write(new int[Materials.Count]);
				for (int i = 0; i < Materials.Count; i++)
				{
					uint off = (uint)dataWriter.Position;
					dataWriter.Write(Materials[i].Write(version, bin.ByteOrder));
					uint endPos = (uint)dataWriter.Position;
					dataWriter.Position = 4 + i * 4;
					dataWriter.Write(off + 8);
					dataWriter.Position = endPos;
				}
				data = newData.ToArray();
				base.WritePane(bin);
			}
		}

		public BasePane RootPane;
		public Grp1Pane RootGroup { get; set; }
		public List<BasePane> Panes = new List<BasePane>();
		public UInt32 version;

		public byte[] SaveFile()
		{
			var res = new MemoryStream();
			BinaryDataWriter bin = new BinaryDataWriter(res);
			bin.ByteOrder = FileByteOrder;
			bin.Write("FLYT", BinaryStringFormat.NoPrefixOrTermination);
			bin.Write((ushort)0xFEFF); //should match 0xFF 0xFE
			bin.Write((UInt16)0x14); //Header size
			bin.Write(version);
			bin.Write((Int32)0);
			UInt16 PaneCount = (UInt16)Panes.Count;
			for (int i = 0; i < Panes.Count; i++)
				if (Panes[i].UserData != null) PaneCount++;
			bin.Write(PaneCount);
			bin.Write((UInt16)0); //padding
			foreach (var p in Panes)
				p.WritePane(bin);
			while (bin.BaseStream.Position % 4 != 0)
				bin.Write((byte)0);
			bin.BaseStream.Position = 0xC;
			bin.Write((uint)bin.BaseStream.Length);
			bin.BaseStream.Position = bin.BaseStream.Length;
			return res.ToArray();
		}

		public TextureSection GetTex
		{
			get
			{
				var res = (TextureSection)Panes.Find(x => x is TextureSection); ;
				if (res == null)
				{
					res = new TextureSection();
					Panes.Insert(2, res);
				}
				return res;
			}
		}

		public MaterialsSection GetMat
		{
			get
			{
				var res = (MaterialsSection)Panes.Find(x => x is MaterialsSection);
				if (res == null)
				{
					res = new MaterialsSection();
					Panes.Insert(3, res);
				}
				return res;
			}
		}

		public BflytFile(byte[] data) : this(new MemoryStream(data)) { }

		public BflytFile(Stream file)
		{
			BinaryDataReader bin = new BinaryDataReader(file);
			FileByteOrder = ByteOrder.LittleEndian;
			bin.ByteOrder = FileByteOrder;
			if (bin.ReadString(4) != "FLYT") throw new Exception("Wrong signature");
			var bOrder = bin.ReadUInt16(); //BOM
			if (bOrder == 0xFFFE)
			{
				FileByteOrder = ByteOrder.BigEndian;
				bin.ByteOrder = FileByteOrder;
			}
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
						Panes.Add(new TextureSection(bin));
						break;
					case "mat1":
						Panes.Add(new MaterialsSection(bin, version));
						break;
					case "usd1":
						Panes.Last().UserData = new Usd1Pane(bin);
						break;
					default:
						var pane = new BasePane(name, bin);
						Panes.Add(DetectProperPaneClass(pane));
						break;
				}
			}

			RebuildParentingData();
		}

		public void RemovePane(BasePane pane)
		{
			int paneIndex = Panes.IndexOf(pane);
			int end = FindPaneEnd(paneIndex);

			Panes.RemoveRange(paneIndex, end - paneIndex + 1);

			if (pane.Parent != null)
				pane.Parent.Children.Remove(pane);
			RebuildParentingData();
		}

		int FindPaneEnd(int paneIndex)
		{
			var pane = Panes[paneIndex];

			string childStarter = pane is Grp1Pane ? "grs1" : "pas1";
			string childCloser = pane is Grp1Pane ? "gre1" : "pae1";

			if (Panes[paneIndex + 1].name == childStarter)
			{
				int ChildLevel = 0;
				int i;
				for (i = paneIndex + 2; i < Panes.Count; i++)
				{
					if (Panes[i].name == childCloser)
					{
						if (ChildLevel == 0)
						{
							break;
						}
						ChildLevel--;
					}
					if (Panes[i].name == childStarter)
						ChildLevel++;
				}
				return i;
			}
			return paneIndex;
		}

		public void AddPane(int offsetInChilren, BasePane Parent, params BasePane[] pane)
		{
			string childStarter = pane[0] is Grp1Pane ? "grs1" : "pas1";
			string childCloser = pane[0] is Grp1Pane ? "gre1" : "pae1";

			if (pane.Length > 1 && (pane[1].name != childStarter || pane[0].Parent == pane[2].Parent))
				throw new Exception("The BasePane array must be a single pane, optionally with children already in the proper structure");

			if (Parent == null) Parent = RootPane;
			int parentIndex = Panes.IndexOf(Parent);
			if (Panes.Count <= parentIndex + 1 || Panes[parentIndex + 1].name != childStarter)
			{
				if (Parent.Children.Count != 0) throw new Exception("Inconsistend data !");
				Panes.Insert(parentIndex + 1, new BasePane(childStarter, 8));
				Panes.Insert(parentIndex + 2, new BasePane(childCloser, 8));
			}

			pane[0].Parent = Parent;
			if (offsetInChilren <= 0 || offsetInChilren >= Parent.Children.Count)
			{
				Parent.Children.AddRange(pane);
				Panes.InsertRange(parentIndex + 2, pane);
			}
			else
			{
				int actualInsertOffset = 0;
				int childCount = 0;
				for (int i = parentIndex + 2; ; i++)
				{
					i = FindPaneEnd(i) + 1;
					childCount++;
					if (childCount == offsetInChilren)
					{
						actualInsertOffset = i;
						break;
					}
				}

				Parent.Children.InsertRange(offsetInChilren, pane);
				Panes.InsertRange(actualInsertOffset, pane);
			}
			RebuildParentingData();
		}

		public void MovePane(BasePane pane, BasePane NewParent, int childOffset)
		{
			if (childOffset < 0)
				childOffset = 0;
			if (childOffset > NewParent.Children.Count)
				childOffset = NewParent.Children.Count;

			int parentIndex = Panes.IndexOf(NewParent);
			if (parentIndex == -1) throw new Exception("No parent !");

			int paneIndex = Panes.IndexOf(pane);

			int paneCount = FindPaneEnd(paneIndex) - paneIndex + 1;

			List<BasePane> tmpForCopy = new List<BasePane>();
			for (int i = paneIndex; i < paneIndex + paneCount; i++)
				tmpForCopy.Add(Panes[i]);

			Panes.RemoveRange(paneIndex, paneCount);

			AddPane(childOffset, NewParent, tmpForCopy.ToArray());

			//RebuildParentingData(); called by AddPane
		}

		void RebuildParentingData()
		{
			RebuildGroupingData();
			BasePane CurrentRoot = null;
			int RootIndex = -1;
			for (int i = 0; i < Panes.Count; i++)
			{
				if (Panes[i] is Pan1Pane && ((Pan1Pane)Panes[i]).PaneName == "RootPane")
				{
					CurrentRoot = Panes[i];
					RootIndex = i;
					break;
				}
			}
			this.RootPane = CurrentRoot ?? throw new Exception("Couldn't find the root pane");
			RootPane.Children.Clear();
			RootPane.Parent = null;
			for (int i = RootIndex + 1; i < Panes.Count; i++)
			{
				if (Panes[i].name == "pas1")
				{
					CurrentRoot = Panes[i - 1];
					CurrentRoot.Children.Clear();
					continue;
				}
				if (Panes[i].name == "pae1")
				{
					CurrentRoot = CurrentRoot.Parent;
					if (CurrentRoot == null) return;
					continue;
				}
				Panes[i].Parent = CurrentRoot;
				CurrentRoot.Children.Add(Panes[i]);
			}
			if (CurrentRoot != null)
				throw new Exception("Unexpected pane data ending: one or more children sections are not closed by the end of the file");
		}

		void RebuildGroupingData()
		{
			int rootGroupIndex = Panes.FindIndex(x => x.name == "grp1");
			var curRoot = Panes[rootGroupIndex] as Grp1Pane;
			RootGroup = curRoot;
			curRoot.Parent = null;
			curRoot.Children.Clear();
			for (int i = rootGroupIndex + 1; i < Panes.Count; i++)
			{
				if (Panes[i].name == "grs1")
				{
					curRoot = (Grp1Pane)Panes[i - 1];
					curRoot.Children.Clear();
					continue;
				}
				else if (Panes[i].name == "gre1")
				{
					curRoot = (Grp1Pane)curRoot.Parent;
					if (curRoot == null) return;
					continue;
				}
				if (!(Panes[i] is Grp1Pane)) break;
				((Grp1Pane)Panes[i]).Parent = curRoot;
				curRoot.Children.Add((Grp1Pane)Panes[i]);
			}
			if (curRoot != RootGroup)
				throw new Exception("Unexpected pane data ending: one or more group sections are not closed by the end of the file");
		}

		BasePane DetectProperPaneClass(BasePane pane)
		{
			switch (pane.name)
			{
				case "pic1":
					return new Pic1Pane(pane, FileByteOrder);
				case "txt1":
					return new Txt1Pane(pane, FileByteOrder);
				case "grp1":
					return new Grp1Pane(pane, FileByteOrder, version);
				default:
					if (pane.data.Length < 0x4C || pane.name == "grp1" || pane.name == "cnt1")
						return pane;
					return new Pan1Pane(pane, FileByteOrder);
			}
		}

		public string[] GetGroupNames() => Panes.Where(x => x is Grp1Pane).Select(x => ((Grp1Pane)x).GroupName).ToArray();
	}
}
