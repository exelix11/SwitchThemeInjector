using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.IO;

namespace SwitchThemes.Common.Bntxx
{
	public class Texture
	{
		#region Header
			public int BRTILength0;
			public long BRTILength1;
			public byte Flags;
			public byte Dimensions;
			public ushort TileMode;
			public ushort SwizzleSize;
			public ushort MipmapCount;
			public ushort MultiSampleCount;
			public ushort Reversed1A;
			public uint Format;
			public uint AccessFlags;
			public int Width;
			public int Height;
			public int Depth;
			public int ArrayCount;
			public int BlockHeightLog2;
			public int Reserved38;
			public int Reserved3C;
			public int Reserved40;
			public int Reserved44;
			public int Reserved48;
			public int Reserved4C;
			public int DataLength;
			public int Alignment;
			public int ChannelTypes;
			public int TextureType;
			public long NameAddress;
			public long ParentAddress;
			public long PtrsAddress;
		#endregion
		public byte[] Data;
		public byte[] ExtraBrtiData;
		public readonly string Name;
		public ChannelType Channel0Type => (ChannelType)((ChannelTypes >> 0) & 0xff);
		public ChannelType Channel3Type => (ChannelType)((ChannelTypes >> 8) & 0xff);
		public ChannelType Channel1Type => (ChannelType)((ChannelTypes >> 16) & 0xff);
		public ChannelType Channel2Type => (ChannelType)((ChannelTypes >> 24) & 0xff);

		public TextureType Type => (TextureType) TextureType;
		public TextureFormatType FormatType =>(TextureFormatType)((Format >> 8) & 0xff);
		public TextureFormatVar  FormatVariant => (TextureFormatVar)((Format >> 0) & 0xff);

		public byte[] Write()
		{
			var mem = new MemoryStream();
			BinaryDataWriter bin = new BinaryDataWriter(mem);
			bin.ByteOrder = ByteOrder.LittleEndian;
			bin.Write("BRTI", BinaryStringFormat.NoPrefixOrTermination);
			bin.Write(BRTILength0);
			bin.Write(BRTILength1);
			bin.Write(Flags);
			bin.Write(Dimensions);
			bin.Write(TileMode);
			bin.Write(SwizzleSize);
			bin.Write(MipmapCount);
			bin.Write(MultiSampleCount);
			bin.Write(Reversed1A);
			bin.Write(Format);
			bin.Write(AccessFlags);
			bin.Write(Width);
			bin.Write(Height);
			bin.Write(Depth);
			bin.Write(ArrayCount);
			bin.Write(BlockHeightLog2);
			bin.Write(Reserved38);
			bin.Write(Reserved3C);
			bin.Write(Reserved40);
			bin.Write(Reserved44);
			bin.Write(Reserved48);
			bin.Write(Reserved4C);
			bin.Write(Data.Length);
			bin.Write(Alignment);
			bin.Write(ChannelTypes);
			bin.Write(TextureType);
			bin.Write(NameAddress);
			bin.Write(ParentAddress);
			bin.Write(PtrsAddress);
			bin.Write(ExtraBrtiData);
			return mem.ToArray();
		}

		public Texture(BinaryDataReader Reader)
		{
			var startPos = Reader.BaseStream.Position;
			if (Reader.ReadString(4) != "BRTI")
				throw new Exception("Wrong magic");

			BRTILength0		= Reader.ReadInt32();
			BRTILength1	= Reader.ReadInt64();
			Flags			= Reader.ReadByte();
			Dimensions		= Reader.ReadByte();
			TileMode		= Reader.ReadUInt16();
			SwizzleSize	= Reader.ReadUInt16();
			MipmapCount	= Reader.ReadUInt16();
			MultiSampleCount = Reader.ReadUInt16();
			Reversed1A	= Reader.ReadUInt16();
			Format			= Reader.ReadUInt32();
			AccessFlags	= Reader.ReadUInt32();
			Width			= Reader.ReadInt32();
			Height			= Reader.ReadInt32();
			Depth			= Reader.ReadInt32();
			ArrayCount		= Reader.ReadInt32();
			BlockHeightLog2 = Reader.ReadInt32();
			Reserved38		= Reader.ReadInt32();
			Reserved3C		= Reader.ReadInt32();
			Reserved40		= Reader.ReadInt32();
			Reserved44		= Reader.ReadInt32();
			Reserved48		= Reader.ReadInt32();
			Reserved4C		= Reader.ReadInt32();
			DataLength		= Reader.ReadInt32();
			Alignment		= Reader.ReadInt32();
			ChannelTypes	= Reader.ReadInt32();
			TextureType		= Reader.ReadInt32();
			NameAddress	= Reader.ReadInt64();
			ParentAddress	= Reader.ReadInt64();
			PtrsAddress	= Reader.ReadInt64();

			ExtraBrtiData = Reader.ReadBytes((int)(BRTILength1 - (Reader.BaseStream.Position - startPos)));

			Reader.BaseStream.Seek(NameAddress, SeekOrigin.Begin);

			Name = Reader.ReadString(BinaryStringFormat.WordLengthPrefix);

			long[] MipOffsets = new long[MipmapCount];

			Reader.BaseStream.Seek(PtrsAddress, SeekOrigin.Begin);

			long BaseOffset = Reader.ReadInt64();

			for (int Mip = 1; Mip < MipmapCount; Mip++)
			{
				throw new Exception("mipmaps are not supported");
				//MipOffsets[Mip] = Reader.ReadInt64() - BaseOffset;
			}

			Reader.BaseStream.Seek(BaseOffset, SeekOrigin.Begin);

			Data = Reader.ReadBytes(DataLength);			
		}
	}	

//    public struct Texture
//    {
//        public string Name;

//        public int Width;
//        public int Height;
//        public int ArrayCount;
//        public int BlockHeightLog2;
//        public int MipmapCount;

//        public long[] MipOffsets;

//        public byte[] Data;

//        public ChannelType Channel0Type;
//        public ChannelType Channel1Type;
//        public ChannelType Channel2Type;
//        public ChannelType Channel3Type;

//        public TextureType       Type;
//        public TextureFormatType FormatType;
//        public TextureFormatVar  FormatVariant;

//        public ISwizzle GetSwizzle()
//        {
//            return new BlockLinearSwizzle(
//                GetWidthInTexels(),
//                GetBytesPerTexel(),
//                GetBlockHeight());
//        }

//        public int GetWidthInTexels()
//        {
//            switch (FormatType)
//            {
//                case TextureFormatType.BC1:
//                case TextureFormatType.BC2:
//                case TextureFormatType.BC3:
//                case TextureFormatType.BC4:
//                case TextureFormatType.BC5:
//                case TextureFormatType.ASTC4x4:
//                    return (Width + 3) / 4;

//                case TextureFormatType.ASTC5x4:
//                case TextureFormatType.ASTC5x5:
//                    return (Width + 4) / 5;

//                case TextureFormatType.ASTC6x5:
//                case TextureFormatType.ASTC6x6:
//                    return (Width + 5) / 6;

//                case TextureFormatType.ASTC8x5:
//                case TextureFormatType.ASTC8x6:
//                case TextureFormatType.ASTC8x8:
//                    return (Width + 7) / 8;

//                case TextureFormatType.ASTC10x5:
//                case TextureFormatType.ASTC10x6:
//                case TextureFormatType.ASTC10x8:
//                case TextureFormatType.ASTC10x10:
//                    return (Width + 9) / 10;

//                case TextureFormatType.ASTC12x10:
//                case TextureFormatType.ASTC12x12:
//                    return (Width + 11) / 12;
//            }

//            return Width;
//        }

//        public int GetPow2HeightInTexels()
//        {
//            int Pow2Height = BitUtils.Pow2RoundUp(Height);

//            switch (FormatType)
//            {
//                case TextureFormatType.BC1:
//                case TextureFormatType.BC2:
//                case TextureFormatType.BC3:
//                case TextureFormatType.BC4:
//                case TextureFormatType.BC5:
//                case TextureFormatType.ASTC4x4:
//                case TextureFormatType.ASTC5x4:
//                    return (Pow2Height + 3) / 4;

//                case TextureFormatType.ASTC5x5:
//                case TextureFormatType.ASTC6x5:
//                case TextureFormatType.ASTC8x5:
//                    return (Pow2Height + 4) / 5;

//                case TextureFormatType.ASTC6x6:
//                case TextureFormatType.ASTC8x6:
//                case TextureFormatType.ASTC10x6:
//                    return (Pow2Height + 5) / 6;

//                case TextureFormatType.ASTC8x8:
//                case TextureFormatType.ASTC10x8:
//                    return (Pow2Height + 7) / 8;

//                case TextureFormatType.ASTC10x10:
//                case TextureFormatType.ASTC12x10:
//                    return (Pow2Height + 9) / 10;

//                case TextureFormatType.ASTC12x12:
//                    return (Pow2Height + 11) / 12;
//            }

//            return Pow2Height;
//        }

//        public int GetBytesPerTexel()
//        {
//            switch (FormatType)
//            {
//                case TextureFormatType.R5G6B5:
//                case TextureFormatType.R8G8:
//                case TextureFormatType.R16:
//                    return 2;

//                case TextureFormatType.R8G8B8A8:
//                case TextureFormatType.R11G11B10:
//                case TextureFormatType.R32:
//                    return 4;

//                case TextureFormatType.BC1:
//                case TextureFormatType.BC4:
//                    return 8;

//                case TextureFormatType.BC2:
//                case TextureFormatType.BC3:
//                case TextureFormatType.BC5:
//                case TextureFormatType.ASTC4x4:
//                case TextureFormatType.ASTC5x4:
//                case TextureFormatType.ASTC5x5:
//                case TextureFormatType.ASTC6x5:
//                case TextureFormatType.ASTC6x6:
//                case TextureFormatType.ASTC8x5:
//                case TextureFormatType.ASTC8x6:
//                case TextureFormatType.ASTC8x8:
//                case TextureFormatType.ASTC10x5:
//                case TextureFormatType.ASTC10x6:
//                case TextureFormatType.ASTC10x8:
//                case TextureFormatType.ASTC10x10:
//                case TextureFormatType.ASTC12x10:
//                case TextureFormatType.ASTC12x12:
//                    return 16;
//            }

//            throw new NotImplementedException();
//        }

//        public int GetBlockHeight()
//        {
//            return 1 << BlockHeightLog2;
//        }

//        public override string ToString()
//        {
//            return Name;
//        }
       
//    }
}
