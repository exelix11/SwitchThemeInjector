using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using ExtensionMethods;
using Syroot.BinaryData;
using System.Linq;
using System.ComponentModel;

namespace SwitchThemes.Common
{
	public class BflanSection
	{
		public string TypeName { get; set; }
		public byte[] Data;

		public BflanSection(string name)
		{
			TypeName = name;
		}

		public BflanSection(string name, byte[] data)
		{
			TypeName = name;
			Data = data;
		}

		public virtual void BuildData(ByteOrder byteOrder)
		{
			return;
		}

		public virtual void Write(BinaryDataWriter bin)
		{
			if (TypeName.Length != 4) throw new Exception("unexpected type len");
			BuildData(bin.ByteOrder);
			bin.Write(TypeName, BinaryStringFormat.NoPrefixOrTermination);
			bin.Write((int)Data.Length + 8);
			bin.Write(Data);
		}
	}

	[TypeConverter(typeof(ExpandableObjectConverter))]
	public class Pat1Section : BflanSection
	{
		public ushort AnimationOrder { get; set; }
		public string Name { get; set; }
		public byte ChildBinding { get; set; }
		public List<string> Groups { get; set; } = new List<string>();

		public UInt16 Unk_StartOfFile { get; set; }
		public UInt16 Unk_EndOfFile { get; set; }
		public byte[] Unk_EndOfHeader { get; set; }

		const int groupNameLen = 0x24;

		public Pat1Section() : base("pat1") { }

		public Pat1Section(byte[] data, ByteOrder bo) : base("pat1", data)
		{
			ParseData(bo);
		}

		void ParseData(ByteOrder bo)
		{
			BinaryDataReader bin = new BinaryDataReader(new MemoryStream(Data));
			bin.ByteOrder = bo;
			AnimationOrder = bin.ReadUInt16();
			var groupCount = bin.ReadUInt16();
			if (groupCount != 1) throw new Exception("File with unexpected group count");
			var animName = bin.ReadUInt32() - 8; //all offsets are shifted by 8 cause this byte block doesn't include the section name and size
			var groupNames = bin.ReadUInt32() - 8;
			Unk_StartOfFile = bin.ReadUInt16();
			Unk_EndOfFile = bin.ReadUInt16();
			ChildBinding = bin.ReadByte();
			Unk_EndOfHeader = bin.ReadBytes((int)animName - (int)bin.Position);
			bin.BaseStream.Position = animName;
			Name = bin.ReadString(BinaryStringFormat.ZeroTerminated);
			for (int i = 0; i < groupCount; i++)
			{
				bin.BaseStream.Position = groupNames + i * groupNameLen;
				Groups.Add(bin.ReadFixedLenString(groupNameLen));
			}
			if (Unk_StartOfFile != 0 || Unk_EndOfFile != 0)
			{
				Console.Write("");
			}
		}

		public override void BuildData(ByteOrder byteOrder)
		{
			MemoryStream mem = new MemoryStream();
			BinaryDataWriter bin = new BinaryDataWriter(mem);
			bin.ByteOrder = byteOrder;
			bin.Write((UInt16)AnimationOrder);
			bin.Write((UInt16)Groups.Count);
			var UpdateOffsetsPos = bin.Position;
			bin.Write((UInt32)0);
			bin.Write((UInt32)0);
			bin.Write(Unk_StartOfFile);
			bin.Write(Unk_EndOfFile);
			bin.Write(ChildBinding);
			bin.Write(Unk_EndOfHeader);
			var oldPos = bin.Position;
			bin.Position = UpdateOffsetsPos;
			bin.Write((uint)oldPos + 8); //name offset
			bin.Position = oldPos;
			bin.Write(Name, BinaryStringFormat.ZeroTerminated);
			while (bin.BaseStream.Position % 4 != 0)
				bin.Write((byte)0);
			oldPos = bin.Position;
			bin.Position = UpdateOffsetsPos + 4; //Group name table
			bin.Write((uint)oldPos + 8);
			bin.Position = oldPos;
			for (int i = 0; i < Groups.Count; i++)
				bin.WriteFixedLenString(Groups[i], groupNameLen);
			Data = mem.ToArray();
		}

		public override string ToString() => "[Pat1 section]";
	}

	[TypeConverter(typeof(ExpandableObjectConverter))]
	public class Pai1Section : BflanSection
	{
		public UInt16 FrameSize { get; set; }
		public byte Flags { get; set; }
		public List<string> Textures { get; set; } = new List<string>();
		public List<PaiEntry> Entries = new List<PaiEntry>();

		[TypeConverter(typeof(ExpandableObjectConverter))]
		public class PaiEntry
		{
			public enum AnimationTarget : byte
			{
				Pane = 0,
				Material = 1
			}

			public string Name { get; set; }
			public AnimationTarget Target { get; set; }
			public List<PaiTag> Tags = new List<PaiTag>();
			public byte[] UnkwnownData { get; set; } = new byte[0];

			public PaiEntry() { }

			public PaiEntry(BinaryDataReader bin)
			{
				uint SectionStart = (uint)bin.Position;
				Name = bin.ReadFixedLenString(28);
				var tagCount = bin.ReadByte();
				Target = (AnimationTarget)bin.ReadByte();
				bin.ReadUInt16(); //padding
				List<uint> TagOffsets = new List<uint>();
				for (int i= 0; i < tagCount; i++)
					TagOffsets.Add(bin.ReadUInt32());
				if (tagCount == 0) return;
				UnkwnownData = bin.ReadBytes((int)(TagOffsets[0] + SectionStart - bin.Position));
				for (int i = 0; i < tagCount; i++)
				{
					bin.BaseStream.Position = TagOffsets[i] + SectionStart;
					Tags.Add(new PaiTag(bin, (byte)Target));
				}
			}

			public void Write(BinaryDataWriter bin)
			{
				uint SectionStart = (uint)bin.Position;
				bin.WriteFixedLenString(Name, 28);
				bin.Write((byte)Tags.Count);
				bin.Write((byte)Target);
				bin.Write((UInt16)0);
				var tagTable = bin.Position;
				for (int i = 0; i < Tags.Count; i++)
					bin.Write((uint)0);
				bin.Write(UnkwnownData);
				for (int i = 0; i < Tags.Count; i++)
				{
					var oldPos = bin.Position;
					bin.Position = tagTable + i * 4;
					bin.Write((uint)oldPos - SectionStart);
					bin.Position = oldPos;
					Tags[i].Write(bin, (byte)Target);
				}
			}
		}

		[TypeConverter(typeof(ExpandableObjectConverter))]
		public class PaiTag
		{
			public uint Unknown { get; set; }
			public string TagType { get; set; }
			public List<PaiTagEntry> Entries = new List<PaiTagEntry>();

			public PaiTag() { }

			public PaiTag(BinaryDataReader bin, byte TargetType)
			{
				if (TargetType == 2)
					Unknown = bin.ReadUInt32(); //This doesn't seem to be included in the offsets to the entries (?)
				var sectionStart = (uint)bin.Position;
				TagType = bin.ReadString(4);
				var entryCount = bin.ReadUInt32();
				List<uint> EntryOffsets = new List<uint>();
				for (int i = 0; i < entryCount; i++)
					EntryOffsets.Add(bin.ReadUInt32());
				for (int i = 0; i < entryCount; i++)
				{
					bin.Position = EntryOffsets[i] + sectionStart;
					Entries.Add(new PaiTagEntry(bin, TagType));
				}
			}

			public void Write(BinaryDataWriter bin, byte TargetType)
			{
				if (TargetType == 2)
					bin.Write(Unknown);
				var sectionStart = (uint)bin.Position;
				bin.Write(TagType, BinaryStringFormat.NoPrefixOrTermination);
				bin.Write((uint)Entries.Count);
				var EntryTable = bin.Position;
				for (int i = 0; i < Entries.Count; i++)
					bin.Write((uint)0);
				for (int i = 0; i < Entries.Count; i++)
				{
					var oldpos = bin.Position;
					bin.Position = EntryTable + i * 4;
					bin.Write((uint)oldpos - sectionStart);
					bin.Position = oldpos;
					Entries[i].Write(bin, TagType);
				}
			}
		}

		[TypeConverter(typeof(ExpandableObjectConverter))]
		public class PaiTagEntry
		{
			public byte Index { get; set; }
			public byte AnimationTarget { get; set; }
			public UInt16 DataType { get; set; }
			public List<KeyFrame> KeyFrames { get; set; } = new List<KeyFrame>();

			public uint FLEUUnknownInt { get; set; } = 0;
			public string FLEUEntryName { get; set; } = "";

			public PaiTagEntry() { }

			public PaiTagEntry(BinaryDataReader bin, string TagName)
			{
				uint tagStart = (uint)bin.Position;
				Index = bin.ReadByte();
				AnimationTarget = bin.ReadByte();
				DataType = bin.ReadUInt16();
				var KeyFrameCount = bin.ReadUInt16();
				bin.ReadUInt16(); //Padding
				bin.BaseStream.Position = tagStart + bin.ReadUInt32(); //offset to first keyframe
				for (int i = 0; i < KeyFrameCount; i++)
					KeyFrames.Add(new KeyFrame(bin, DataType));
				if (TagName == "FLEU")
				{
					FLEUUnknownInt = bin.ReadUInt32();
					FLEUEntryName = bin.ReadString(BinaryStringFormat.ZeroTerminated);
				}
			}

			public void Write(BinaryDataWriter bin, string TagName)
			{
				uint tagStart = (uint)bin.Position;
				bin.Write(Index);
				bin.Write(AnimationTarget);
				bin.Write(DataType);
				bin.Write((UInt16)KeyFrames.Count);
				bin.Write((UInt16)0);
				bin.Write((uint)bin.Position - tagStart + 4);
				for (int i = 0; i < KeyFrames.Count; i++)
				{
					bin.Write(KeyFrames[i].Frame);
					if (DataType == 2)
					{
						bin.Write(KeyFrames[i].Value);
						bin.Write(KeyFrames[i].Blend);
					}
					else if (DataType == 1)
					{
						bin.Write((Int16)KeyFrames[i].Value);
						bin.Write((Int16)KeyFrames[i].Blend);
					}
					else throw new Exception("Unexpected data type for KeyFrame");
				}
				if (TagName == "FLEU")
				{
					bin.Write(FLEUUnknownInt);
					bin.Write(FLEUEntryName, BinaryStringFormat.ZeroTerminated);
					while (bin.BaseStream.Position % 4 != 0)
						bin.Write((byte)0);
				}
			}
		}

		[TypeConverter(typeof(ExpandableObjectConverter))]
		public class KeyFrame
		{
			public float Frame { get; set; }
			public float Value { get; set; }
			public float Blend { get; set; }
			public KeyFrame(BinaryDataReader bin, UInt16 DataType)
			{
				Frame = bin.ReadSingle();
				if (DataType == 2)
				{
					Value = bin.ReadSingle();
					Blend = bin.ReadSingle();
				}
				else if (DataType == 1)
				{
					Value = (float)bin.ReadInt16();
					Blend = (float)bin.ReadInt16();
				}
				else throw new Exception("Unexpected data type for keyframe");
			}

			public KeyFrame() { }
		}

		public Pai1Section() : base("pai1") { }

		public Pai1Section(byte[] data, ByteOrder bo) : base("pai1", data)
		{
			ParseData(bo);
		}

		void ParseData(ByteOrder bo)
		{
			BinaryDataReader bin = new BinaryDataReader(new MemoryStream(Data));
			bin.ByteOrder = bo;
			FrameSize = bin.ReadUInt16();
			Flags = bin.ReadByte();
			bin.ReadByte(); //padding
			var texCount = bin.ReadUInt16();
			var entryCount = bin.ReadUInt16();
			var entryTable = bin.ReadUInt32() - 8;
			if (texCount != 0)
			{
				var texTableStart = bin.Position;
				List<uint> offsets = new List<uint>();
				for (int i = 0; i < texCount; i++)
					offsets.Add(bin.ReadUInt32());
				for (int i = 0; i < texCount; i++)
				{
					bin.Position = texTableStart + offsets[i];
					Textures.Add(bin.ReadString(BinaryStringFormat.ZeroTerminated));
				}
			}
			for (int i = 0; i < entryCount; i++)
			{
				bin.Position = entryTable + i * 4;
				bin.Position = bin.ReadUInt32() - 8;
				Entries.Add(new PaiEntry(bin));
			}
		}

		public override void BuildData(ByteOrder byteOrder)
		{
			MemoryStream mem = new MemoryStream();
			BinaryDataWriter bin = new BinaryDataWriter(mem);
			bin.ByteOrder = byteOrder;
			bin.Write(FrameSize);
			bin.Write(Flags);
			bin.Write((byte)0);
			bin.Write((UInt16)Textures.Count);
			bin.Write((UInt16)Entries.Count);
			var updateOffsets = bin.Position;
			bin.Write((uint)0);
			if (Textures.Count != 0)
			{
				var texTableStart = bin.Position;
				bin.Write(new byte[Textures.Count * 4]); //make space for tex offsets

				for (int i = 0; i < Textures.Count; i++)
				{
					var texPos = bin.Position;
					bin.Write(Textures[i], BinaryStringFormat.ZeroTerminated);
					var endPos = bin.Position;
					bin.Position = texTableStart + i * 4;
					bin.Write((UInt32)(texPos - texTableStart));
					bin.Position = endPos;
				}

				while (bin.BaseStream.Position % 4 != 0)
					bin.Write((byte)0);
			}
			var EntryTableStart = bin.Position;
			bin.Position = updateOffsets;
			bin.Write((uint)EntryTableStart + 8);
			bin.Position = EntryTableStart;
			for (int i = 0; i < Entries.Count; i++)
				bin.Write((uint)0);

			for (int i = 0; i < Entries.Count; i++)
			{
				var oldpos = bin.Position;
				bin.Position = EntryTableStart + 4 * i;
				bin.Write((uint)oldpos + 8);
				bin.Position = oldpos;
				Entries[i].Write(bin);
			}

			Data = mem.ToArray();
		}

		public override string ToString() => "[Pai1 section]";
	}

	public class Bflan
	{
		public ByteOrder byteOrder { get; set; }
		public uint Version { get; set; }

		public List<BflanSection> Sections = new List<BflanSection>();

		[Browsable(false)]
		public Pat1Section patData => Sections.Where(x => x is Pat1Section).FirstOrDefault() as Pat1Section;
		[Browsable(false)]
		public Pai1Section paiData => Sections.Where(x => x is Pai1Section).FirstOrDefault() as Pai1Section;

		public Bflan() { }

		public Bflan(byte[] data) => ParseFile(new MemoryStream(data));

		public byte[] WriteFile()
		{
			MemoryStream mem = new MemoryStream();
			BinaryDataWriter bin = new BinaryDataWriter(mem);
			bin.ByteOrder = byteOrder;
			bin.Write("FLAN", BinaryStringFormat.NoPrefixOrTermination);
			bin.Write((UInt16)0xFEFF);
			bin.Write((UInt16)0x14);
			bin.Write(Version);
			bin.Write((uint)0); //Filesize
			bin.Write((UInt16)Sections.Count);
			bin.Write((UInt16)0);

			for (int i = 0; i < Sections.Count; i++)
				Sections[i].Write(bin);

			bin.Position = 0xC;
			bin.Write((uint)bin.BaseStream.Length);
			return mem.ToArray();
		}

		void ParseFile(Stream input)
		{
			var bin = new BinaryDataReader(input);
			if (!bin.ReadBytes(4).Matches("FLAN"))
				throw new Exception("Wrong bflan magic");
			byte BOM = bin.ReadByte();
			if (BOM == 0xFF) byteOrder = ByteOrder.LittleEndian;
			else if (BOM == 0xFE) byteOrder = ByteOrder.BigEndian;
			else throw new Exception("Unexpected BOM");
			bin.ByteOrder = byteOrder;
			bin.ReadByte(); //Second byte of the byte order mask
			if (bin.ReadUInt16() != 0x14) throw new Exception("Unexpected bflan header size");
			Version = bin.ReadUInt32();
			bin.ReadUInt32(); //FileSize
			var sectionCount = bin.ReadUInt16();
			bin.ReadUInt16(); //padding ?

			for (int i = 0; i < sectionCount; i++)
			{
				string sectionName = bin.ReadString(4);
				int sectionSize = bin.ReadInt32(); //this includes the first 8 bytes we read here
				byte[] sectionData = bin.ReadBytes(sectionSize - 8);
				BflanSection s = null;
				switch (sectionName)
				{
					case "pat1":
						s = new Pat1Section(sectionData, bin.ByteOrder);
						break;
					case "pai1":
						s = new Pai1Section(sectionData, bin.ByteOrder);
						break;
					default:
						throw new Exception("unexpected section");
				}
				Sections.Add(s);
			}
		}

	}
}
