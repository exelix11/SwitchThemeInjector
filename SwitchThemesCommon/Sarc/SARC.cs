using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using Syroot.BinaryData;
using ExtensionMethods;

/* My SARC library. Packing is not yet supported. */

namespace SARCExt
{
	public class SarcData
	{
		public Dictionary<string, byte[]> Files;
		public ByteOrder endianness;
		public bool HashOnly;
	}

    public static class SARC
    {
        static uint NameHash(string name)
        {
            uint result = 0;
            for (int i = 0; i < name.Length; i++)
            {
                result = name[i] + result * 0x00000065;
            }
            return result;
        }

		static uint StringHashToUint(string name)
		{
			if (name.Contains("."))
				name = name.Split('.')[0];
			if (name.Length != 8) throw new Exception("Invalid hash length");
			return Convert.ToUInt32(name, 16);
		}

		public static string GuessFileExtension(byte[] f)
        {
            string Ext = ".bin";

            if (f.Matches("SARC")) Ext = ".sarc";
            else if (f.Matches("Yaz")) Ext = ".szs";
            else if (f.Matches("YB") || f.Matches("BY")) Ext = ".byaml";
            else if (f.Matches("FRES")) Ext = ".bfres";
            else if (f.Matches("Gfx2")) Ext = ".gtx";
            else if (f.Matches("FLYT")) Ext = ".bflyt";
            else if (f.Matches("CLAN")) Ext = ".bclan";
            else if (f.Matches("CLYT")) Ext = ".bclyt";
            else if (f.Matches("FLIM")) Ext = ".bclim";
            else if (f.Matches("FLAN")) Ext = ".bflan";
            else if (f.Matches("FSEQ")) Ext = ".bfseq";
            else if (f.Matches("VFXB")) Ext = ".pctl";
            else if (f.Matches("AAHS")) Ext = ".sharc";
            else if (f.Matches("BAHS")) Ext = ".sharcb";
            else if (f.Matches("BNTX")) Ext = ".bntx";
            else if (f.Matches("BNSH")) Ext = ".bnsh";
            else if (f.Matches("FSHA")) Ext = ".bfsha";
            else if (f.Matches("FFNT")) Ext = ".bffnt";
            else if (f.Matches("CFNT")) Ext = ".bcfnt";
            else if (f.Matches("CSTM")) Ext = ".bcstm";
            else if (f.Matches("FSTM")) Ext = ".bfstm";
            else if (f.Matches("STM")) Ext = ".bfsha";
            else if (f.Matches("CWAV")) Ext = ".bcwav";
            else if (f.Matches("FWAV")) Ext = ".bfwav";
            else if (f.Matches("CTPK")) Ext = ".ctpk";
            else if (f.Matches("CGFX")) Ext = ".bcres";
            else if (f.Matches("AAMP")) Ext = ".aamp";
            else if (f.Matches("MsgStdBn")) Ext = ".msbt";
            else if (f.Matches("MsgPrjBn")) Ext = ".msbp";
            return Ext;
        }

		public static uint GuessAlignment(Dictionary<string, byte[]> files) //From https://github.com/aboood40091/SarcLib/blob/master/src/FileArchive.py#L487
		{
			uint res = 4;
			foreach (var f in files.Values)
			{
				uint fileRes = GuessFileAlignment(f);				
				res = fileRes > res ? fileRes : res;
			}
			return res;
		}

		public static uint GuessFileAlignment(byte[] f)
		{
			if (f.Matches("SARC")) return 0x2000;
			else if (f.Matches("Yaz")) return 0x80;
			else if (f.Matches("YB") || f.Matches("BY")) return 0x80;
			else if (f.Matches("FRES") || f.Matches("Gfx2") || f.Matches("AAHS") || f.Matches("BAHS")) return 0x2000;
			else if (f.Matches("BNTX") || f.Matches("BNSH") || f.Matches("FSHA")) return 0x1000;
			else if (f.Matches("FFNT")) return 0x2000;
			else if (f.Matches("CFNT")) return 0x80;
			else if (f.Matches(1, "STM") /* *STM */ || f.Matches(1, "WAV") || f.Matches("FSTP")) return 0x20;
			else if (f.Matches("CTPK")) return 0x10;
			else if (f.Matches("CGFX")) return 0x80;
			else if (f.Matches("AAMP")) return 8;
			else if (f.Matches("MsgStdBn") || f.Matches("MsgPrjBn")) return 0x80;
			else return 0x4;
		}

		public static Tuple<int, byte[]> PackN(SarcData data, int _align = -1)
		{
			int align = _align >= 0 ? _align : (int)GuessAlignment(data.Files);

			MemoryStream o = new MemoryStream();
			BinaryDataWriter bw = new BinaryDataWriter(o, false);
			bw.ByteOrder = data.endianness;
			bw.Write("SARC", BinaryStringFormat.NoPrefixOrTermination);
			bw.Write((UInt16)0x14); // Chunk length
			bw.Write((UInt16)0xFEFF); // BOM
			bw.Write((UInt32)0x00); //filesize update later
			bw.Write((UInt32)0x00); //Beginning of data
			bw.Write((UInt16)0x100);
			bw.Write((UInt16)0x00);
			bw.Write("SFAT", BinaryStringFormat.NoPrefixOrTermination);
			bw.Write((UInt16)0xc);
			bw.Write((UInt16)data.Files.Keys.Count);
			bw.Write((UInt32)0x00000065);
			List<uint> offsetToUpdate = new List<uint>();

			//Names must be sorted by hash
			string[] SortedNames = null;
			if (data.HashOnly)
				SortedNames = data.Files.Keys.OrderBy(x => StringHashToUint(x)).ToArray();
			else
				SortedNames = data.Files.Keys.OrderBy(x => NameHash(x)).ToArray();

			foreach (string k in SortedNames)
			{
				if (data.HashOnly)
					bw.Write(StringHashToUint(k));
				else
					bw.Write(NameHash(k));
				offsetToUpdate.Add((uint)bw.BaseStream.Position);
				bw.Write((UInt32)0);
				bw.Write((UInt32)0);
				bw.Write((UInt32)0);
			}
			bw.Write("SFNT", BinaryStringFormat.NoPrefixOrTermination);
			bw.Write((UInt16)0x8);
			bw.Write((UInt16)0);
			List<uint> StringOffsets = new List<uint>();
			foreach (string k in SortedNames)
			{
				StringOffsets.Add((uint)bw.BaseStream.Position);
				bw.Write(k, BinaryStringFormat.ZeroTerminated);
				bw.Align(4);
			}
			bw.Align(0x1000); //TODO: check if works in odyssey
			List<uint> FileOffsets = new List<uint>();
			foreach (string k in SortedNames)
			{
				bw.Align((int)GuessFileAlignment(data.Files[k]));
				FileOffsets.Add((uint)bw.BaseStream.Position);
				bw.Write(data.Files[k]);
			}
			for (int i = 0; i < offsetToUpdate.Count; i++)
			{
				bw.BaseStream.Position = offsetToUpdate[i];
				if (!data.HashOnly)
					bw.Write(0x01000000 | ((StringOffsets[i] - StringOffsets[0]) / 4));
				else
					bw.Write((UInt32)0);
				bw.Write((UInt32)(FileOffsets[i] - FileOffsets[0]));
				bw.Write((UInt32)(FileOffsets[i] + data.Files.Values.ToArray()[i].Length - FileOffsets[0]));
			}
			bw.BaseStream.Position = 0x08;
			bw.Write((uint)bw.BaseStream.Length);
			bw.Write((uint)FileOffsets[0]);

			return new Tuple<int, byte[]>(align, o.ToArray());
		}

		public static SarcData UnpackRamN(byte[] src) =>
			UnpackRamN(new MemoryStream(src));

		public static SarcData UnpackRamN(Stream src)
		{
			Dictionary<string, byte[]> res = new Dictionary<string, byte[]>();
			BinaryDataReader br = new BinaryDataReader(src, false);
			br.ByteOrder = ByteOrder.LittleEndian;
			br.BaseStream.Position = 6;
			if (br.ReadUInt16() == 0xFFFE)
				br.ByteOrder = ByteOrder.BigEndian;
			br.BaseStream.Position = 0;
			if (br.ReadString(4) != "SARC")
				throw new Exception("Wrong magic");

			br.ReadUInt16(); // Chunk length
			br.ReadUInt16(); // BOM
			br.ReadUInt32(); // File size
			UInt32 startingOff = br.ReadUInt32();
			br.ReadUInt32(); // Unknown;
			SFAT sfat = new SFAT();
			sfat.parse(br, (int)br.BaseStream.Position);
			SFNT sfnt = new SFNT();
			sfnt.parse(br, (int)br.BaseStream.Position, sfat, (int)startingOff);

			bool HashOnly = false;
			if (sfat.nodeCount > 0)
			{
				if (sfat.nodes[0].fileBool != 1) HashOnly = true;
			}

			for (int m = 0; m < sfat.nodeCount; m++)
			{
				br.Seek(sfat.nodes[m].nodeOffset + startingOff, 0);
				byte[] temp;
				if (m == 0)
				{
					temp = br.ReadBytes((int)sfat.nodes[m].EON);
				}
				else
				{
					int tempInt = (int)sfat.nodes[m].EON - (int)sfat.nodes[m].nodeOffset;
					temp = br.ReadBytes(tempInt);
				}
				if (sfat.nodes[m].fileBool == 1)
					res.Add(sfnt.fileNames[m], temp);
				else
					res.Add(sfat.nodes[m].hash.ToString("X8") + GuessFileExtension(temp), temp);
			}

			return new SarcData() {endianness = br.ByteOrder, HashOnly = HashOnly, Files = res };
		}

		[Obsolete("This has been kept for compatibility, use PackN instead")]
		public static byte[] pack(Dictionary<string, byte[]> files, int align = -1, ByteOrder endianness = ByteOrder.LittleEndian) =>
			PackN(new SarcData() { Files = files, endianness = endianness, HashOnly = false }, align).Item2;

		[Obsolete("This has been kept for compatibility, use UnpackRamN instead")]
		public static Dictionary<string, byte[]> UnpackRam(byte[] src) =>
			UnpackRamN(new MemoryStream(src)).Files;

		[Obsolete("This has been kept for compatibility, use UnpackRamN instead")]
		public static Dictionary<string, byte[]> UnpackRam(Stream src) =>
			UnpackRamN(src).Files;
				
       public class SFAT
        {
            public List<Node> nodes = new List<Node>();
            
            public UInt16 chunkSize;
            public UInt16 nodeCount;
            public UInt32 hashMultiplier;

            public struct Node
            {
                public UInt32 hash;
                public byte fileBool;
                public byte unknown1;
                public UInt16 fileNameOffset;
                public UInt32 nodeOffset;
                public UInt32 EON;
            }

            public void parse(BinaryDataReader br, int pos)
            {
                br.ReadUInt32(); // Header;
                chunkSize = br.ReadUInt16();
                nodeCount = br.ReadUInt16();
                hashMultiplier = br.ReadUInt32();
                for (int i = 0; i < nodeCount; i++)
                {
                    Node node;
                    node.hash = br.ReadUInt32();
					//node.fileBool = br.ReadByte();
					//node.unknown1 = br.ReadByte();
					//node.fileNameOffset = br.ReadUInt16();
					var attributes = br.ReadUInt32();
					node.fileBool = (byte)(attributes >> 24);
					node.unknown1 = (byte)((attributes >> 16) & 0xFF);
					node.fileNameOffset = (UInt16)(attributes & 0xFFFF);
					node.nodeOffset = br.ReadUInt32();
                    node.EON = br.ReadUInt32();
                    nodes.Add(node);
                }
            }

        }

        public class SFNT
        {
            public List<string> fileNames = new List<string>();

            public UInt32 chunkID;
            public UInt16 chunkSize;
            public UInt16 unknown1;
            
            public void parse(BinaryDataReader br, int pos, SFAT sfat, int start)
            {
                chunkID = br.ReadUInt32();
                chunkSize = br.ReadUInt16();
                unknown1 = br.ReadUInt16();

                char[] temp = br.ReadChars(start - (int)br.BaseStream.Position);
                string temp2 = new string(temp);
                char[] splitter = { (char)0x00 };
                string[] names = temp2.Split(splitter);
                for (int j = 0; j < names.Length; j++)
                {
                    if (names[j] != "")
                    {
                        fileNames.Add(names[j]);
                    }
                }
            }
        }
    }
}
