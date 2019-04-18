using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using ExtensionMethods;
using Syroot.BinaryData;
using static SwitchThemes.Common.BflytFile;

namespace SwitchThemes.Common
{
	public class Grp1Pane : BasePanel
	{
		public string GroupName { get; set; }
		public List<string> Panes { get; set; } = new List<string>();

		public uint Version;

		public override string ToString() => GroupName + " [Group]";

		void LoadProperties()
		{
			BinaryDataReader bin = new BinaryDataReader(new MemoryStream(data));
			bin.ByteOrder = ByteOrder.LittleEndian;
			if (Version > 0x05020000)
				GroupName = bin.ReadFixedLenString(34);
			else
				GroupName = bin.ReadFixedLenString(24);
			var NodeCount = bin.ReadUInt16();
			if (Version <= 0x05020000)
				bin.ReadUInt16();
			var pos = bin.Position;
			for (int i = 0; i < NodeCount; i++)
			{
				bin.Position = pos + i * 24;
				Panes.Add(bin.ReadFixedLenString(24));
			}
		}

		public Grp1Pane(BinaryDataReader bin, uint version) : base("grp1", bin)
		{
			Version = version;
			LoadProperties();
		}

		public Grp1Pane(BasePanel p, uint version) : base(p)
		{
			Version = version;
			LoadProperties();
		}

		public Grp1Pane(uint version) : base("grp1", 8) { Version = version; }

		void ApplyChanges()
		{
			MemoryStream mem = new MemoryStream();
			BinaryDataWriter bin = new BinaryDataWriter(mem);
			if (Version > 0x05020000)
				bin.WriteFixedLenString(GroupName, 34);
			else
				bin.WriteFixedLenString(GroupName, 24);
			bin.Write((UInt16)Panes.Count);
			if (Version <= 0x05020000)
				bin.Write((UInt16)0);
			foreach (var s in Panes)
				bin.WriteFixedLenString(s, 24);
			data = mem.ToArray();
		}

		public override void WritePanel(BinaryDataWriter bin)
		{
			ApplyChanges();
			base.WritePanel(bin);
		}
	}
}
