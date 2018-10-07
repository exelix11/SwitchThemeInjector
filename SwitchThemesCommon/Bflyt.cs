using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SwitchThemes.Common
{
	class BflytFile
	{
		public class BasePanel
		{
			public override string ToString()
			{
				return $"Panel {name} len: 0x{length.ToString("X")}";
			}

			public string name;
			public Int32 length;
			public byte[] data;

			public BasePanel(string _name, int len)
			{
				name = _name;
				length = len;
				data = new byte[length - 8];
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
			public List<byte[]> Materials = new List<byte[]>();
			public MaterialsSection(BinaryDataReader bin) : base("mat1", bin)
			{
				BinaryDataReader dataReader = new BinaryDataReader(new MemoryStream(data));
				dataReader.ByteOrder = bin.ByteOrder;
				int matCount = dataReader.ReadInt32();
				var Offsets = dataReader.ReadInt32s(matCount).Select(x => x - 8).ToArray(); // offsets relative to the stream
				for (int i = 0; i < matCount; i++)
				{
					int matLen = (i == matCount - 1 ? (int)dataReader.BaseStream.Length : Offsets[i + 1]) - (int)dataReader.Position;
					Materials.Add(dataReader.ReadBytes(matLen));
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
					dataWriter.Write(Materials[i]);
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
			public string PanelName;
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

		public void PatchTextureName(string original, string _new)
		{
			var texSection = GetTex;
			for (int i = 0; i < texSection.Textures.Count; i++)
			{
				if (texSection.Textures[i] == original)
				{
					texSection.Textures[i] = _new;
				}
			}
		}

		PatchResult AddBgPanel(int index, string TexName, string Pic1Name)
		{
			#region add picture
			if (Pic1Name.Length > 0x18)
				throw new Exception("Pic1Name should not be longer than 24 chars");
			var BgPanel = new BasePanel("pic1", 0x8);
			Panels.Insert(index, BgPanel);
			var MatSect = GetMat;
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
				bin.Write((UInt16)MatSect.Materials.Count);
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
					MatSect.Materials.Add(mem.ToArray());
				}
			}
			#endregion
			return PatchResult.OK;
		}

		public PatchResult PatchMainLayout(PatchTemplate patch)
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
					if (!patch.NoRemovePanel)
					{
						using (BinaryDataWriter bin = new BinaryDataWriter(new MemoryStream(Panels[i].data)))
						{
							bin.ByteOrder = ByteOrder.LittleEndian;
							bin.BaseStream.Position = 0x24;
							bin.Write(5000f);
							bin.Write(60000f);
							Panels[target].data = ((MemoryStream)bin.BaseStream).ToArray();
						}
					}
				}
			}
			if (target == int.MaxValue) return PatchResult.Fail;
			#endregion
			return AddBgPanel(target, patch.MaintextureName, patch.PatchIdentifier);
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
						Panels.Add(new MaterialsSection(bin));
						break;
					case "pic1":
						Panels.Add(new PicturePanel(bin));
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
