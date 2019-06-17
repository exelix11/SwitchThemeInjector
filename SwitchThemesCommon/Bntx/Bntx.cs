using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SwitchThemes.Common.Bntxx
{
	class QuickBntx
	{
		public QuickBntx(BinaryDataReader Reader) 
		{
			if (Reader.ReadString(4) != "BNTX")
				throw new Exception("Wrong magic");

			Reader.ReadInt32();
			int DataLength = Reader.ReadInt32();
			ushort ByteOrderMark = Reader.ReadUInt16();
			ushort FormatRevision = Reader.ReadUInt16();
			int NameAddress = Reader.ReadInt32();
			int StringsAddress = Reader.ReadInt32() >> 16;
			int RelocAddress = Reader.ReadInt32();
			int FileLength = Reader.ReadInt32();

			if (Reader.ReadString(4) != "NX  ")
				throw new Exception("Wrong magic");

			uint TexturesCount = Reader.ReadUInt32();
			long InfoPtrsAddress = Reader.ReadInt64();
			long DataBlkAddress = Reader.ReadInt64();
			long DictAddress = Reader.ReadInt64();
			uint StrDictLength = Reader.ReadUInt32();

			Reader.BaseStream.Seek(InfoPtrsAddress, SeekOrigin.Begin);
			var FirstBrti = (int)Reader.ReadInt64();
			Reader.BaseStream.Position = 0;
			Head = Reader.ReadBytes(FirstBrti);

			for (int Index = 0; Index < TexturesCount; Index++)
			{
				Reader.BaseStream.Seek(InfoPtrsAddress + Index * 8, SeekOrigin.Begin);
				Reader.BaseStream.Seek(Reader.ReadInt64(), SeekOrigin.Begin);

				Textures.Add(new Texture(Reader));
				//File.WriteAllBytes("F:\\test", Textures[0].Write());
			}

			Reader.BaseStream.Position = RelocAddress;
			Rlt = Reader.ReadBytes((int)Reader.BaseStream.Length - (int)Reader.BaseStream.Position);
		}

		//this will work only for a BC1 image, other formats are not implemented, sizes different than 720p haven't been tested
		public void ReplaceTex(string texName, byte[] DDS)
		{
			var dds = DDSEncoder.LoadDDS(DDS);
			ReplaceTex(texName, dds);
		}

		public Bntxx.Texture FindTex(string texName) => Textures.Where(x => x.Name == texName).FirstOrDefault();

		public void ReplaceTex(string texName, DDSEncoder.DDSLoadResult dds)
		{
			var target = Textures.Where(x => x.Name == texName).First();
			var encoded = DDSEncoder.EncodeTex(dds);
			target.Data = encoded.Data;
			target.TextureType = (int)TextureType.Image2D;
			target.Format = (uint)encoded.format.formatCode;
			target.ChannelTypes = 0x05040302;
			target.Width = dds.width;
			target.Height = dds.height;
			target.TileMode = 0;
			target.SwizzleSize = 0;
			target.Reversed1A = 0;
			target.Reserved4C = 0;
			target.Reserved48 = 0;
			target.Reserved44 = 0;
			target.Reserved40 = 0;
			target.Reserved3C = 0;
			target.Reserved38 = 0x00010007;
			target.MipmapCount = 1;
			target.Flags = 0x01;
			target.Depth = 1;
			target.BlockHeightLog2 = encoded.blockHeightLog2;
			target.Alignment = 0x200;
			target.AccessFlags = 0x20;
		}

		public List<Texture> Textures = new List<Texture>();
		byte[] Head;
		public byte[] Rlt;
		public byte[] Write()
		{
			var mem = new MemoryStream();
			BinaryDataWriter bin = new BinaryDataWriter(mem);
			bin.ByteOrder = ByteOrder.LittleEndian;
			bin.Write(Head);
			List<long> TexPositions = new List<long>();
			foreach (var t in Textures)
			{
				TexPositions.Add(bin.BaseStream.Position);
				bin.Write(t.Write());
			}
			var DataStart = bin.BaseStream.Position;
			List<long> TexDataPositions = new List<long>();
			bin.Align(0x10);
			bin.Write("BRTD", BinaryStringFormat.NoPrefixOrTermination);
			bin.Write((int)0);
			bin.Write((int)0);
			bin.Write((int)0);
			foreach (var t in Textures)
			{
				TexDataPositions.Add(bin.BaseStream.Position);
				bin.Write(t.Data);
				bin.Align(0x10);
			}
			bin.Align(0x1000);
			UInt32 rltPos = (UInt32)bin.BaseStream.Position;
			bin.Write(Rlt);
			//Update offsets
			bin.BaseStream.Position = 0x18;
			bin.Write((UInt32)rltPos);
			bin.Write((UInt32)bin.BaseStream.Length);
			bin.BaseStream.Position = TexDataPositions[0] - 8;
			bin.Write(rltPos - (TexDataPositions[0] - 0x10));
			for (int i = 0; i < TexPositions.Count; i++)
			{
				bin.BaseStream.Position = TexPositions[i] + 0x2A0;
				bin.Write(TexDataPositions[i]);
			}
			bin.BaseStream.Position = rltPos + 4;
			bin.Write(rltPos);
			return mem.ToArray();
		}
	}
}
