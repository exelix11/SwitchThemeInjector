using ExtensionMethods;
using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Globalization;
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
}

namespace SwitchThemes.Common.Bflyt
{
	public class BflytFile
	{
		public IEnumerable<BasePane> EnumeratePanes() => EnumeratePanes(RootPanes);

		public IEnumerable<BasePane> EnumeratePanes(BasePane source) => EnumeratePanes(new List<BasePane>() { source });

		public IEnumerable<BasePane> EnumeratePanes(List<BasePane> source)
		{
			var ToProcess = new Queue<BasePane>(RootPanes);

			while (ToProcess.Count > 0)
			{
				BasePane item = ToProcess.Dequeue();
				yield return item;
				foreach (var c in item.Children)
					ToProcess.Enqueue(c);
			}
		}

		public BasePane FindPane(Func<BasePane, bool> condition) => EnumeratePanes().Where(condition).FirstOrDefault();

		public BasePane this[string name]
		{
			get => FindPane(x => (x as INamedPane)?.PaneName == name);
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
			public byte[] data;

			public byte[] GetData() => data;

			public BasePane(string _name, int len)
			{
				name = _name;
				data = new byte[len - 8];
			}

			public BasePane(string _name, BinaryDataReader bin)
			{
				name = _name;
				var length = bin.ReadInt32();
				data = bin.ReadBytes(length - 8);
			}

			public BasePane(string _name, byte[] _data)
			{
				name = _name;
				data = _data;
			}

			protected virtual void ApplyChanges(BinaryDataWriter bin) { }

			public void WritePane(BinaryDataWriter bin)
			{
				var mem = new MemoryStream();
				using (BinaryDataWriter b = new BinaryDataWriter(mem))
				{
					b.ByteOrder = bin.ByteOrder;
					ApplyChanges(b);
					if (b.BaseStream.Length != 0)
					{
						//bin.Align(4);
						data = mem.ToArray();
					}
				}

				bin.Write(name, BinaryStringFormat.NoPrefixOrTermination);
				bin.Write((UInt32)(data.Length + 8));
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

			protected override void ApplyChanges(BinaryDataWriter dataWriter)
			{
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

			protected override void ApplyChanges(BinaryDataWriter dataWriter)
			{
				dataWriter.Write((Int16)Materials.Count);
				dataWriter.Write((Int16)0); //padding
				dataWriter.Write(new int[Materials.Count]);
				for (int i = 0; i < Materials.Count; i++)
				{
					uint off = (uint)dataWriter.Position;
					dataWriter.Write(Materials[i].Write(version, dataWriter.ByteOrder));
					uint endPos = (uint)dataWriter.Position;
					dataWriter.Position = 4 + i * 4;
					dataWriter.Write(off + 8);
					dataWriter.Position = endPos;
				}
			}
		}

		public List<BasePane> RootPanes = new List<BasePane>();
		public Pan1Pane ElementsRoot => FindPane(x => x is Pan1Pane) as Pan1Pane;
		public Grp1Pane RootGroup => RootPanes.Find(x => x is Grp1Pane) as Grp1Pane;

		public UInt32 Version;

		private List<BasePane> WritePaneListForBinary()
		{
			List<BasePane> res = new List<BasePane>();
			void RecursivePushPane(BasePane p)
			{
				res.Add(p);
				if (p.Children.Count != 0)
				{
					string childStarter = p is Grp1Pane ? "grs1" : "pas1";
					string childCloser = p is Grp1Pane ? "gre1" : "pae1";

					res.Add(new BasePane(childStarter, 8));
					foreach (var c in p.Children)
						RecursivePushPane(c);
					res.Add(new BasePane(childCloser, 8));
				}
			}

			foreach (var r in RootPanes)
				RecursivePushPane(r);

			return res;
		}

		public byte[] SaveFile()
		{
			var Panes = WritePaneListForBinary();

			var res = new MemoryStream();
			BinaryDataWriter bin = new BinaryDataWriter(res);
			bin.ByteOrder = FileByteOrder;
			bin.Write("FLYT", BinaryStringFormat.NoPrefixOrTermination);
			bin.Write((ushort)0xFEFF); //should match 0xFF 0xFE
			bin.Write((UInt16)0x14); //Header size
			bin.Write(Version);
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

		//Getters will not add the section if it's missing
		public TextureSection GetTex
		{
			get => RootPanes.Find(x => x is TextureSection) as TextureSection;
		}

		public TextureSection GetTexturesSection()
		{
			if (GetTex != null) return GetTex;
			//the textures section is often after a fnl1 section
			var res = new TextureSection();
			var fnt = RootPanes.Find(x => x.name == "fnl1");
			if (fnt != null)
				RootPanes.Insert(RootPanes.IndexOf(fnt) + 1, res);
			else RootPanes.Insert(1, res);
			return res;
		}

		public MaterialsSection GetMat
		{
			get => RootPanes.Find(x => x is MaterialsSection) as MaterialsSection;
		}

		public MaterialsSection GetMaterialsSection()
		{
			if (GetMat != null) return GetMat;
			//the materials section is often after the txl1 section
			var res = new MaterialsSection();
			var tex = GetTex;
			if (tex == null)
			{
				var fnt = RootPanes.Find(x => x.name == "fnl1");
				if (fnt != null)
					RootPanes.Insert(RootPanes.IndexOf(fnt) + 1, res);
				else RootPanes.Insert(1, res);
			}
			else RootPanes.Insert(RootPanes.IndexOf(tex) + 1, res);
			return res;
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
			Version = bin.ReadUInt32();
			bin.ReadUInt32(); //File size
			var sectionCount = bin.ReadUInt16();
			bin.ReadUInt16(); //padding

			BasePane lastPane = null;
			Stack<BasePane> currentRoot = new Stack<BasePane>();
			void PushPane(BasePane p)
			{
				if (p.name == "pas1" || p.name == "grs1")
					currentRoot.Push(lastPane);
				else if (p.name == "pae1" || p.name == "gre1")
					currentRoot.Pop();
				else if (currentRoot.Count == 0)
					RootPanes.Add(p);
				else
				{
					p.Parent = currentRoot.Peek();
					currentRoot.Peek().Children.Add(p);
				}

				lastPane = p;
			}

			for (int i = 0; i < sectionCount; i++)
			{
				string name = bin.ReadString(4);
				switch (name)
				{
					case "txl1":
						PushPane(new TextureSection(bin));
						break;
					case "mat1":
						PushPane(new MaterialsSection(bin, Version));
						break;
					case "usd1":
						lastPane.UserData = new Usd1Pane(bin);
						break;
					case "pic1":
						PushPane(new Pic1Pane(bin));
						break;
					case "txt1":
						PushPane(new Txt1Pane(bin));
						break;
					case "grp1":
						PushPane(new Grp1Pane(bin, Version));
						break;
					case "pan1":	case "prt1":
					case "wnd1":	case "bnd1":
						PushPane(new Pan1Pane(bin, name));
						break;
					default:
						PushPane(new BasePane(name, bin));
						break;
				}
			}
		}

		public void RemovePane(BasePane pane)
		{
			pane.Parent.Children.Remove(pane);
		}

		public void AddPane(int offset, BasePane Parent, BasePane pane)
		{
			if (offset < 0) offset = 0;
			if (offset > Parent.Children.Count) offset = Parent.Children.Count;
			Parent.Children.Insert(offset, pane);
		}

		public void MovePane(BasePane pane, BasePane NewParent, int offset)
		{
			RemovePane(pane);
			AddPane(offset, NewParent, pane);
		}
	}
}
