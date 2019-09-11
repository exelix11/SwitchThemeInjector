//TODO: not complete, before release this has to be implemented in the differ and in SwitchThemes injector and installer

using ExtensionMethods;
using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Text;
using static SwitchThemes.Common.Bflyt.BflytFile;

namespace SwitchThemes.Common.Bflyt
{
	public class Txt1Pane : Pan1Pane
	{
		public UInt16 TextLength { get; set; }
		public UInt16 RestrictedTextLength { get; set; }
		public UInt16 MaterialIndex { get; set; }
		public UInt16 FontIndex { get; set; }

		byte TextAlign;
		public OriginX HorizontalAlignment
		{
			get => (OriginX)((TextAlign >> 2) & 0x3);
			set
			{
				TextAlign &= unchecked((byte)(~0xC));
				TextAlign |= (byte)((byte)(value) << 2);
			}
		}

		public OriginX VerticalAlignment
		{
			get => (OriginX)((TextAlign) & 0x3);
			set
			{
				TextAlign &= unchecked((byte)(~0x3));
				TextAlign |= (byte)(value);
			}
		}

		public enum LineAlign : byte
		{
			Unspecified = 0,
			Left = 1,
			Center = 2,
			Right = 3,
		};
		public LineAlign LineAlignment { get; set; }

		byte flags;
		public bool PerCharTransform
		{
			get => (flags & 0x10) != 0;
			set => flags = value ? (byte)(flags | 0x10) : unchecked((byte)(flags & (~0x10)));
		}
		public bool RestrictedTextLengthEnabled
		{
			get => (flags & 0x2) != 0;
			set => flags = value ? (byte)(flags | 0x2) : unchecked((byte)(flags & (~0x2)));
		}
		public bool ShadowEnabled
		{
			get => (flags & 1) != 0;
			set => flags = value ? (byte)(flags | 1) : unchecked((byte)(flags & (~1)));
		}

		public enum BorderType : byte
		{
			Standard = 0,
			DeleteBorder = 1,
			RenderTwoCycles = 2,
		};

		public BorderType BorderFormat
		{
			get => (BorderType)((flags >> 2) & 0x3);
			set
			{
				flags &= unchecked((byte)(~0xC));
				flags |= (byte)((byte)value << 2);
			}
		}

		public float ItalicTilt { get; set; }

		public RGBAColor FontTopColor { get; set; }
		public RGBAColor FontBottomColor { get; set; }
		public Vector2 FontXYSize { get; set; }
		public float CharacterSpace { get; set; }
		public float LineSpace { get; set; }

		public float[] ShadowXY { get; set; } = new float[0];
		public float[] ShadowXYSize { get; set; } = new float[0];
		public RGBAColor ShadowTopColor { get; set; }
		public RGBAColor ShadowBottomColor { get; set; }
		public float ShadowItalic { get; set; }

		public string Text { get; internal set; }

		public Txt1Pane(ByteOrder b) : base("txt1", b, 0xA4) { }

		public Txt1Pane(byte[] data, ByteOrder b) : base(data, "txt1", b)
		{
			ParseData();
		}

		public Txt1Pane(BinaryDataReader bin) : base(bin, "txt1")
        {
			ParseData();
        }

		private void ParseData()
		{
			BinaryDataReader dataReader = new BinaryDataReader(new MemoryStream(data));
			dataReader.ByteOrder = order;
			dataReader.Position = 0x54 - 8;
			TextLength = dataReader.ReadUInt16();
			RestrictedTextLength = dataReader.ReadUInt16();
			MaterialIndex = dataReader.ReadUInt16();
			FontIndex = dataReader.ReadUInt16();
			TextAlign = dataReader.ReadByte();
			LineAlignment = (LineAlign)dataReader.ReadByte();
			flags = dataReader.ReadByte();
			dataReader.ReadByte(); //padding
			ItalicTilt = dataReader.ReadSingle();
			uint TextOffset = dataReader.ReadUInt32();
			FontTopColor = dataReader.ReadColorRGBA();
			FontBottomColor = dataReader.ReadColorRGBA();
			FontXYSize = dataReader.ReadVector2();
			CharacterSpace = dataReader.ReadSingle();
			LineSpace = dataReader.ReadSingle();
			uint TbNameOffset = dataReader.ReadUInt32();
			ShadowXY = dataReader.ReadSingles(2);
			ShadowXYSize = dataReader.ReadSingles(2);
			ShadowTopColor = dataReader.ReadColorRGBA();
			ShadowBottomColor = dataReader.ReadColorRGBA();
			ShadowItalic = dataReader.ReadSingle();
			dataReader.Position = TextOffset - 8;
			Text = dataReader.ReadString(BinaryStringFormat.ZeroTerminated, Encoding.Unicode);
		}

        protected override void ApplyChanges(BinaryDataWriter bin)
        {
            base.ApplyChanges(bin);
            bin.Write(TextLength);
            bin.Write(RestrictedTextLength);
            bin.Write(MaterialIndex);
            bin.Write(FontIndex);
			bin.Write((byte)TextAlign);
			bin.Write((byte)LineAlignment);
			bin.Write((byte)flags);
			bin.Write((byte)0);
			bin.Write(ItalicTilt);
			bin.BaseStream.Position += 4; //Skip text offset
			bin.Write(FontTopColor);
            bin.Write(FontBottomColor);
            bin.Write(FontXYSize);
            bin.Write(CharacterSpace);
            bin.Write(LineSpace);
			bin.BaseStream.Position += 4; //Skip name offset
            bin.Write(ShadowXY);
            bin.Write(ShadowXYSize);
            bin.Write(ShadowTopColor);
            bin.Write(ShadowBottomColor);
            bin.Write(ShadowItalic);
        }

		public override BasePane Clone() =>
			new Txt1Pane(base.Clone().GetData(), order);
	}
}
