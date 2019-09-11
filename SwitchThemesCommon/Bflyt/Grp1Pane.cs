using ExtensionMethods;
using SwitchThemes.Common;
using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Text;
using static SwitchThemes.Common.Bflyt.BflytFile;

namespace SwitchThemes.Common.Bflyt
{
	public class Grp1Pane : BasePane, INamedPane
	{
		public string GroupName { get; set; }
		public string PaneName => GroupName;

#if LYTEDITOR
		[Editor(@"System.Windows.Forms.Design.StringCollectionEditor," +
	  "System.Design, Version=2.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a",
	 typeof(System.Drawing.Design.UITypeEditor))]
#endif
		public List<string> Panes { get; set; } = new List<string>();

		public uint Version;

		public override string ToString() => GroupName + " [Group]";

		public Grp1Pane(uint ver) : base("grp1", 8)
		{
			Version = ver;
		}

		public Grp1Pane(BinaryDataReader src, uint version) : base("grp1", src)
		{
			BinaryDataReader bin = new BinaryDataReader(new MemoryStream(data));
			bin.ByteOrder = src.ByteOrder;
			Version = version;
			UInt16 NodeCount = 0;
			if (version >= 0x05020000) //3dskit docs say this should be > but files from mario maker for wii u use 34 bytes for names with version 5.2
			{
				GroupName = bin.ReadFixedLenString(34);
				NodeCount = bin.ReadUInt16();
			}
			else
			{
				GroupName = bin.ReadFixedLenString(24);
				NodeCount = bin.ReadUInt16();
				bin.ReadUInt16(); //padding
			}
			var pos = bin.Position;
			for (int i = 0; i < NodeCount; i++)
			{
				bin.Position = pos + i * 24;
				Panes.Add(bin.ReadFixedLenString(24));
			}
		}

		protected override void ApplyChanges(BinaryDataWriter bin)
		{
			if (Version >= 0x05020000)
			{
				bin.WriteFixedLenString(GroupName, 34);
				bin.Write((UInt16)Panes.Count);
			}
			else
			{
				bin.WriteFixedLenString(GroupName, 24);
				bin.Write((UInt16)Panes.Count);
				bin.Write((UInt16)0);
			}
			foreach (var s in Panes)
				bin.WriteFixedLenString(s, 24);
		}
	}
}
