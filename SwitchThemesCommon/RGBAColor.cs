using System;
using System.Collections.Generic;
#if LYTEDITOR
using System.ComponentModel;
using System.Drawing.Design;
using System.Globalization;
#endif

//This class is used to avoid having to link with System.Drawing when compiling for platforms that don't need to provide editor UIs like the web injector
//define LYTEDITOR to enable support for Drawing.Color
namespace SwitchThemes.Common
{
#if LYTEDITOR
	class ArgbConverter : System.Drawing.ColorConverter
	{ 
		public override bool GetStandardValuesSupported(
				ITypeDescriptorContext context)
		{
			return false;
		}

		public override bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType) => 
			sourceType == typeof(string);
		public override bool CanConvertTo(ITypeDescriptorContext context, Type destinationType) =>
			destinationType == typeof(string);

		public override object ConvertFrom(ITypeDescriptorContext context, CultureInfo culture, object value)
		{
			string[] s = ((string)value).Split(';');
			if (s.Length < 3 || s.Length > 4) throw new Exception("Parse error");
			if (s.Length == 3)
				return new RGBAColor(byte.Parse(s[0]), byte.Parse(s[1]), byte.Parse(s[2]));
			else
				return new RGBAColor(byte.Parse(s[0]), byte.Parse(s[1]), byte.Parse(s[2]), byte.Parse(s[3]));		
		}

		public override object ConvertTo(ITypeDescriptorContext context, CultureInfo culture, object value, Type destinationType) =>
			((RGBAColor)value).ToString();
	}

	class RGBAEditor : System.Drawing.Design.ColorEditor
	{
		public override object EditValue(ITypeDescriptorContext context, IServiceProvider provider, object value)
		{
			var col = base.EditValue(context, provider, ((RGBAColor)value).Color);
			return new RGBAColor((System.Drawing.Color)col);
		}

		public override void PaintValue(PaintValueEventArgs e)
		{
			PaintValueEventArgs evt = null;
			if (e.Value is RGBAColor)
				evt = new PaintValueEventArgs(e.Context, ((RGBAColor)e.Value).Color, e.Graphics, e.Bounds);
			else //handle the case e.Value is a System.Drawing.Color
				evt = new PaintValueEventArgs(e.Context, e.Value, e.Graphics, e.Bounds);
			base.PaintValue(evt);
		}
	}

	[TypeConverter(typeof(ArgbConverter))]
	[Editor(typeof(RGBAEditor),typeof(System.Drawing.Design.UITypeEditor))]
#endif
	public class RGBAColor : IEquatable<RGBAColor>
	{
		public byte R, G, B, A;
		public RGBAColor(byte r, byte g, byte b, byte a = 255)
		{
			R = r; G = g; B = b; A = a;
		}

		public RGBAColor(string LeByteString)
		{
			uint Col = Convert.ToUInt32(LeByteString, 16);
			R = (byte)(Col & 0xFF);
			G = (byte)((Col >> 8) & 0xFF);
			B = (byte)((Col >> 16) & 0xFF);
			A = (byte)((Col >> 24) & 0xFF);
		}

		public override string ToString() => A == 255 ? $"{R};{G};{B}" : $"{R};{G};{B};{A}";

#if LYTEDITOR
		public RGBAColor(System.Drawing.Color c)
		{
			Color = c;
		}

		public System.Drawing.Color Color {
			get => System.Drawing.Color.FromArgb(A, R, G, B);
			set
			{
				R = value.R;
				G = value.G;
				B = value.B;
				A = value.A;
			}
		}

		public static explicit operator System.Drawing.Color(RGBAColor d) => d.Color;
		public static explicit operator RGBAColor(System.Drawing.Color c) => new RGBAColor(c.R,c.G,c.B,c.A);
#endif

		public string AsHexLEString() =>
			((uint)(R | G << 8 | B << 16 | A << 24)).ToString("X8");

		public override int GetHashCode()
		{
			var hashCode = 1960784236;
			hashCode = hashCode * -1521134295 + R.GetHashCode();
			hashCode = hashCode * -1521134295 + G.GetHashCode();
			hashCode = hashCode * -1521134295 + B.GetHashCode();
			hashCode = hashCode * -1521134295 + A.GetHashCode();
			return hashCode;
		}

		public override bool Equals(object obj)
		{
#if LYTEDITOR
			if (obj is System.Drawing.Color)
				return (System.Drawing.Color)obj == Color;
#endif
			return Equals(obj as RGBAColor);
		}

		public bool Equals(RGBAColor other)
		{
			return other != null &&
				   R == other.R &&
				   G == other.G &&
				   B == other.B &&
				   A == other.A;
		}

		public static bool operator ==(RGBAColor left, RGBAColor right) =>
			EqualityComparer<RGBAColor>.Default.Equals(left, right);

		public static bool operator !=(RGBAColor left, RGBAColor right) => !(left == right);
	}
}
