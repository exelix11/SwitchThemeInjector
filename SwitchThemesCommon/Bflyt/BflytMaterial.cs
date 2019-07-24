using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.IO;
using ExtensionMethods;
using System.ComponentModel;
using System.Linq;

namespace SwitchThemes.Common.Bflyt
{
	public class BflytMaterial : IInspectable
	{
		[TypeConverter(typeof(ExpandableObjectConverter))]
		public struct TextureReference
		{
			public override string ToString() => $"{{Texture reference}}";

			public enum WRAPS : byte
			{
				NearClamp = 0,
				NearRepeat = 1,
				NearMirror = 2,
				GX2MirrorOnce = 3,
				Clamp = 4,
				Repeat = 5,
				Mirror = 6,
				GX2MirrorOnceBorder = 7
			}

			public UInt16 TextureId { get; set; }
			public WRAPS WrapS { get; set; }
			public WRAPS WrapT { get; set; }
		}

		[TypeConverter(typeof(ExpandableObjectConverter))]
		public struct TextureTransofrm
		{
			public override string ToString() => $"transform ({X},{Y}) ({ScaleX}, {ScaleY}) {Rotation}";

			public float X { get; set; }
			public float Y { get; set; }
			public float Rotation { get; set; }
			public float ScaleX { get; set; }
			public float ScaleY { get; set; }
		}

		public override bool Equals(object obj)
		{
			if (obj == null) return false;
			if (obj is BflytMaterial)
				return Data.SequenceEqual(((BflytMaterial)obj).Data);
			return false;
		}

		byte[] Data;
		public byte[] GetData() => Data;
		Int32 bitflags;

		string _name = "";
		public string Name
		{
			get => _name;
			set
			{
				if (value.Length > 27) throw new Exception("This name is too long");
				_name = value;
			}
		}

		public Color ForegroundColor { get; set; }
		public Color BackgroundColor { get; set; }
		
		//TODO: finish the implementation
		//public bool HasAlphaComparisonConditions { get; set; }
		//public bool HasIndirectAdjustment { get; set; }
		//public bool HasShadowBlending { get; set; }

		public TextureReference[] Textures { get; set; }
		public TextureTransofrm[] TextureTransformations { get; set; }

		public BflytMaterial(byte[] data, ByteOrder bo, uint version)
		{
			Data = data;
			BinaryDataReader bin = new BinaryDataReader(new MemoryStream(data));
			bin.ByteOrder = bo;
			Name = bin.ReadFixedLenString(28); //this string should be null terminated, the actual len should be 27
			if (version >= 0x08000000)
			{
				bitflags = bin.ReadInt32();
				bin.ReadUInt32();
				ForegroundColor = bin.ReadColorRGBA();
				BackgroundColor = bin.ReadColorRGBA();
			}
			else
			{
				ForegroundColor = bin.ReadColorRGBA();
				BackgroundColor = bin.ReadColorRGBA();
				bitflags = bin.ReadInt32();
			}
			Textures = new TextureReference[bitflags & 3];
			for (int i = 0; i < (bitflags & 3); i++)
			{
				Textures[i] = new TextureReference()
				{
					TextureId = bin.ReadUInt16(),
					WrapS = (TextureReference.WRAPS)bin.ReadByte(),
					WrapT = (TextureReference.WRAPS)bin.ReadByte()
				};
			}
			TextureTransformations = new TextureTransofrm[(bitflags & 0xC) >> 2];
			for (int i = 0; i < ((bitflags & 0xC) >> 2); i++)
			{
				TextureTransformations[i] = new TextureTransofrm()
				{
					X = bin.ReadSingle(),
					Y = bin.ReadSingle(),
					Rotation = bin.ReadSingle(),
					ScaleX = bin.ReadSingle(),
					ScaleY = bin.ReadSingle()
				};
			}
		}

		public byte[] Write(uint version, ByteOrder _bo)
		{
			if (Textures.Length > 3) throw new Exception($"[{Name}] A material can have no more than 3 texture references");
			if (TextureTransformations.Length > 3) throw new Exception($"[{Name}] A material can have no more than 3 texture transformations");

			bitflags &= ~3;
			bitflags |= Textures.Length;

			bitflags &= ~0xC;
			bitflags |= TextureTransformations.Length << 2;

			var mem = new MemoryStream();
			BinaryDataWriter bin = new BinaryDataWriter(mem);
			bin.ByteOrder = _bo;
			bin.Write(Data);

			bin.BaseStream.Position = 0;
			bin.WriteFixedLenString(Name, 28);
			if (version >= 0x08000000)
			{
				bin.Write(bitflags);
				bin.BaseStream.Position += 4;
				bin.Write(ForegroundColor);
				bin.Write(BackgroundColor);
			}
			else
			{
				bin.Write(ForegroundColor);
				bin.Write(BackgroundColor);
				bin.Write(bitflags);
			}

			for (int i = 0; i < Textures.Length; i++)
			{
				bin.Write(Textures[i].TextureId);
				bin.Write((byte)Textures[i].WrapS);
				bin.Write((byte)Textures[i].WrapT);
			}

			for (int i = 0; i < TextureTransformations.Length; i++)
			{
				var t = TextureTransformations[i];
				bin.Write(t.X);
				bin.Write(t.Y);
				bin.Write(t.Rotation);
				bin.Write(t.ScaleX);
				bin.Write(t.ScaleY);
			}

			return mem.ToArray();
		}

		public override string ToString() => Name;
	}
}
