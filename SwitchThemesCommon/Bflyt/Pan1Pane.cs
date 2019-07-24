using ExtensionMethods;
using SwitchThemes.Common;
using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using static SwitchThemes.Common.Bflyt.BflytFile;

namespace SwitchThemes.Common.Bflyt
{
	public class Pan1Pane : BasePane, INamedPane
	{
		public CusRectangle transformedRect
		{
			get
			{
				if (Alpha == 0 || !ParentVisibility)
					return new CusRectangle(0, 0, 0, 0);

				Vector2 ParentSize;

				if (Parent != null && Parent is Pan1Pane)
					ParentSize = ((Pan1Pane)Parent).Size;
				else
					ParentSize = new Vector2(0, 0);

				float RelativeX;
				if (ParentOriginX == OriginX.Center) RelativeX = 0;
				else if (ParentOriginX == OriginX.Right) RelativeX = ParentSize.X;
				else RelativeX = ParentSize.X / 2;

				float RelativeY;
				if (ParentOriginY == OriginY.Center) RelativeY = 0;
				else if (ParentOriginY == OriginY.Bottom) RelativeY = ParentSize.Y;
				else RelativeY = ParentSize.Y / 2;

				if (originX == OriginX.Center) RelativeX -= Size.X / 2;
				else if (originX == OriginX.Right) RelativeX -= Size.X;

				if (originY == OriginY.Center) RelativeY -= Size.Y / 2;
				else if (originY == OriginY.Bottom) RelativeY -= Size.Y;

				return new CusRectangle(
					(int)((RelativeX)),
					(int)((RelativeY)),
					(int)(Size.X),
					(int)(Size.Y));
			}
		}

		//This is not an actual property, it's just to hide it from the view
		public bool ViewInEditor { get; set; } = true;

		public bool ParentVisibility
		{
			get
			{
				if (Scale.X == 0 || Scale.Y == 0)
					return false;
				if (!Visible)
					return false;
				if (Parent != null && Parent is Pan1Pane)
				{
					return ((Pan1Pane)Parent).ParentVisibility && Visible;
				}
				return true;
			}
		}

		public Vector2 ParentScale
		{
			get
			{
				if (Parent != null && Parent is Pan1Pane)
				{
					return ((Pan1Pane)Parent).ActualScale;
				}
				return new Vector2(1, 1);
			}
		}

		public Vector2 ActualScale
		{
			get
			{
				if (Parent != null && Parent is Pan1Pane)
				{
					var pScale = ((Pan1Pane)Parent).ActualScale;
					return new Vector2(pScale.X * Scale.X, pScale.Y * Scale.Y);
				}
				return Scale;
			}
		}

		public enum OriginX : byte
		{
			Center = 0,
			Left = 1,
			Right = 2
		};

		public enum OriginY : byte
		{
			Center = 0,
			Top = 1,
			Bottom = 2
		};

		public override string ToString()
		{
			return $"{PaneName} [{name}]";
		}

		byte _flag1;
		byte _flag2;
		public byte Alpha { get; set; } = 255;
		public byte Unknown1;
		public string PaneName { get; set; }
		public readonly string UserInfo;
		public Vector3 Position { get; set; }
		public Vector3 Rotation { get; set; }
		public Vector2 Scale { get; set; } = new Vector2(1, 1);
		public Vector2 Size { get; set; } = new Vector2(100, 100);

		protected ByteOrder order;

		public bool Visible
		{
			get => (_flag1 & 0x1) == 0x1;
			set
			{
				if (value)
					_flag1 |= 0x1;
				else
					_flag1 &= 0xFE;
			}
		}

		public bool InfluenceAlpha
		{
			get => (_flag1 & 0x2) == 0x2;
			set
			{
				if (value)
					_flag1 |= 0x2;
				else
					_flag1 &= 0xFD;
			}
		}

		public OriginX originX
		{
			get => (OriginX)((_flag2 & 0xC0) >> 6);
			set
			{
				_flag2 &= unchecked((byte)(~0xC0));
				_flag2 |= (byte)((byte)value << 6);
			}
		}

		public OriginY originY
		{
			get => (OriginY)((_flag2 & 0x30) >> 4);
			set
			{
				_flag2 &= unchecked((byte)(~0x30));
				_flag2 |= (byte)((byte)value << 4);
			}
		}

		public OriginX ParentOriginX
		{
			get => (OriginX)((_flag2 & 0xC) >> 2);
			set
			{
				_flag2 &= unchecked((byte)(~0xC));
				_flag2 |= (byte)((byte)value << 2);
			}
		}

		public OriginY ParentOriginY
		{
			get => (OriginY)((_flag2 & 0x3));
			set
			{
				_flag2 &= unchecked((byte)(~0x3));
				_flag2 |= (byte)value;
			}
		}

		//public uint[] ColorData = null; //only for pic1 panes

		public Pan1Pane(string paneName, ByteOrder b, int size = 0x54) : base(paneName, size)
		{
			order = b;
			PaneName = "new pane";
			MemoryStream mem = new MemoryStream();
			BinaryDataWriter dataWriter = new BinaryDataWriter(mem);
			dataWriter.ByteOrder = b;
			ApplyChanges(dataWriter);
			data = mem.ToArray();
			Visible = true;
		}

		public Pan1Pane(BasePane p, ByteOrder _order) : base(p)
		{
			order = _order;
			BinaryDataReader dataReader = new BinaryDataReader(new MemoryStream(data));
			dataReader.ByteOrder = order;

			string ReadBinaryString(int max)
			{
				string res = "";
				for (int i = 0; i < max; i++)
				{
					var c = (char)dataReader.ReadByte();
					if (c == 0) continue;
					res += c;
				}
				return res;
			}

			_flag1 = dataReader.ReadByte();
			_flag2 = dataReader.ReadByte();
			Alpha = dataReader.ReadByte();
			Unknown1 = dataReader.ReadByte();
			PaneName = ReadBinaryString(0x18);
			UserInfo = ReadBinaryString(0x8);
			Position = dataReader.ReadVector3();
			Rotation = dataReader.ReadVector3();
			Scale = dataReader.ReadVector2();
			Size = dataReader.ReadVector2();
		}

		protected override void ApplyChanges(BinaryDataWriter bin)
		{
			void WriteBinaryString(string s, int max)
			{
				if (s.Length > max) throw new Exception("The string is longer than the field");
				bin.Write(s, BinaryStringFormat.NoPrefixOrTermination);
				for (int i = s.Length; i < max; i++)
					bin.Write((byte)0);
			}

			bin.Write(data);
			bin.BaseStream.Position = 0;
			bin.Write(_flag1);
			bin.Write(_flag2);
			bin.Write(Alpha);
			bin.Write(Unknown1);
			WriteBinaryString(PaneName, 0x18);
			bin.BaseStream.Position = 0x2C - 8;
			bin.Write(Position);
			bin.Write(Rotation);
			bin.Write(Scale);
			bin.Write(Size);
		}

		public override void WritePane(BinaryDataWriter bin)
		{
			using (var mem = new MemoryStream())
			{
				BinaryDataWriter dataWriter = new BinaryDataWriter(mem);
				dataWriter.ByteOrder = bin.ByteOrder;
				ApplyChanges(dataWriter);
				data = mem.ToArray();
			}
			base.WritePane(bin);
		}

		public override BasePane Clone()
		{
			return new Pan1Pane(base.Clone(),order);
		}
	}
}
