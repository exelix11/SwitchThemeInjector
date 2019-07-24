using ExtensionMethods;
using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Text;
using static SwitchThemes.Common.Bflyt.BflytFile;

namespace SwitchThemes.Common.Bflyt
{
	public class Pic1Pane : Pan1Pane
	{
		public Color ColorTopRight { get; set; } 
		public Color ColorTopLeft { get; set; } 
		public Color ColorBottomRight { get; set; }
		public Color ColorBottomLeft { get; set; } 

		public UInt16 MaterialIndex { get; set; } = 0;

		[TypeConverter(typeof(ExpandableObjectConverter))]
		public class UVCoord
		{
			public Vector2 TopLeft { get; set; }
			public Vector2 TopRight { get; set; }
			public Vector2 BottomLeft { get; set; }
			public Vector2 BottomRight { get; set; }
		}
		public UVCoord[] UVCoords { get; set; }

		public Pic1Pane(ByteOrder b) : base("pic1",b, 0x68) {	}

		public Pic1Pane(BasePane p, ByteOrder b) : base(p, b)
		{
			BinaryDataReader dataReader = new BinaryDataReader(new MemoryStream(data));
			dataReader.ByteOrder = b;
			dataReader.Position = 0x54 - 8;
			ColorTopLeft = dataReader.ReadColorRGBA();
			ColorTopRight = dataReader.ReadColorRGBA();
			ColorBottomLeft = dataReader.ReadColorRGBA();
			ColorBottomRight = dataReader.ReadColorRGBA();
			MaterialIndex = dataReader.ReadUInt16();
			byte UVCount = dataReader.ReadByte();
			dataReader.ReadByte(); //padding
			UVCoords = new UVCoord[UVCount];
			for (int i = 0; i < UVCount; i++)
			{
				UVCoords[i] = new UVCoord()
				{
					TopLeft = dataReader.ReadVector2(),
					TopRight = dataReader.ReadVector2(),
					BottomLeft = dataReader.ReadVector2(),
					BottomRight = dataReader.ReadVector2()
				};
			}
		}

		protected override void ApplyChanges(BinaryDataWriter bin)
		{
			base.ApplyChanges(bin);
			bin.Write(ColorTopLeft);
			bin.Write(ColorTopRight);
			bin.Write(ColorBottomLeft);
			bin.Write(ColorBottomRight);
			bin.Write(MaterialIndex);
			bin.Write((byte)UVCoords.Length);
			bin.Write((byte)0);
			for (int i = 0; i < UVCoords.Length; i++)
			{
				bin.Write(UVCoords[i].TopLeft);
				bin.Write(UVCoords[i].TopRight);
				bin.Write(UVCoords[i].BottomLeft);
				bin.Write(UVCoords[i].BottomRight);
			}
		}

		public override BasePane Clone()
		{
			return new Pic1Pane(base.Clone(),order);
		}
	}
}
