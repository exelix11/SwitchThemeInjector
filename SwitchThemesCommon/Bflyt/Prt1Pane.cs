using ExtensionMethods;
using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace SwitchThemes.Common.Bflyt
{
	public class Prt1Pane : Pan1Pane
	{
		public class Prt1Section 
		{
			public string Name { get; set; }
			public byte Unknown { get; set; }
			public byte Flags { get; set; }
			public UInt16 Padding { get; set; }
			public UInt32 SubpaneOffset { get; set; }
			public UInt32 ComplementOffset { get; set; }
			public UInt32 ExtraOffset { get; set; }
		}

		uint Version;

		public Prt1Pane(byte[] data, ByteOrder b, uint version) : base(data, "prt1", b)
		{
			Version = version;
			ParseData();
		}

		public Prt1Pane(BinaryDataReader bin, uint version) : base(bin, "prt1")
		{
			Version = version;
			ParseData();
		}

		public Vector2 SectionsSacle { get; set; }
		public Prt1Section[] Entries { get; set; }
		public string PartName { get; set; }

		private void ParseData() 
		{
			BinaryDataReader bin = new BinaryDataReader(new MemoryStream(data));
			bin.ByteOrder = order;
			bin.Position = 0x54 - 8;

			UInt32 entriesCount = bin.ReadUInt32();
			SectionsSacle = bin.ReadVector2();

			Entries = new Prt1Section[entriesCount];
			for (UInt32 i = 0; i < entriesCount; i++)
			{
				Entries[i] = new Prt1Section();
				Entries[i].Name = bin.ReadFixedLenString(24);
				Entries[i].Unknown = bin.ReadByte();
				Entries[i].Flags = bin.ReadByte();
				Entries[i].Padding = bin.ReadUInt16();
				Entries[i].SubpaneOffset = bin.ReadUInt32();
				Entries[i].ComplementOffset = bin.ReadUInt32();
				Entries[i].ExtraOffset = bin.ReadUInt32();
			}

			if (Version >= 0x08000000)
				PartName = bin.ReadFixedLenString(24);
		}
	}
}
