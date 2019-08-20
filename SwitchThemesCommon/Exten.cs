using SwitchThemes.Common;
using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.Text;

namespace ExtensionMethods
{
	public static class Extensions
	{
		public static Color ReadColorRGBA(this BinaryDataReader bin)
		{
			byte[] values = bin.ReadBytes(4);
			return new Color(values[0], values[1], values[2], values[3]);
		}

		public static void Write(this BinaryDataWriter bin, Color c)
		{
			bin.Write(c.R);
			bin.Write(c.G);
			bin.Write(c.B);
			bin.Write(c.A);
		}

		public static Vector3 ReadVector3(this BinaryDataReader bin) =>
			new Vector3 { X = bin.ReadSingle(), Y = bin.ReadSingle(), Z = bin.ReadSingle() };

		public static void Write(this BinaryDataWriter bin, Vector3 vec)
		{
			bin.Write((float)vec.X);
			bin.Write((float)vec.Y);
			bin.Write((float)vec.Z);
		}

		public static Vector2 ReadVector2(this BinaryDataReader bin) =>
			new Vector2 { X = bin.ReadSingle(), Y = bin.ReadSingle() };

		public static void Write(this BinaryDataWriter bin, Vector2 vec)
		{
			bin.Write((float)vec.X);
		    bin.Write((float)vec.Y);
		}

		public static bool Matches(this byte[] arr, string magic) =>
			arr.Matches(0, Encoding.ASCII.GetBytes(magic));
		public static bool Matches(this byte[] arr, uint startIndex, string magic) =>
			arr.Matches(startIndex, Encoding.ASCII.GetBytes(magic));

		public static bool Matches(this byte[] arr, uint startIndex, params byte[] magic)
		{
			if (arr.Length < magic.Length + startIndex) return false;
			for (uint i = 0; i < magic.Length; i++)
			{
				if (arr[i + startIndex] != magic[i]) return false;
			}
			return true;
		}

		public static bool ContainsStr(this string[] arr, string t)
		{
			for (int i = 0; i < arr.Length; i++)
				if (arr[i] == t) return true;
			return false;
		}

		public static void WriteFixedLenString(this BinaryDataWriter bin, string s, int max)
		{
			if (s.Length > max) throw new Exception("The string is longer than the field");
			bin.Write(s, BinaryStringFormat.NoPrefixOrTermination);
			for (int i = s.Length; i < max; i++)
				bin.Write((byte)0);
		}

		public static string ReadFixedLenString(this BinaryDataReader bin, int max) =>
			bin.ReadFixedLenString(max, Encoding.UTF8);

		public static string ReadFixedLenString(this BinaryDataReader bin, int max, Encoding encoding)
		{
			var data = bin.ReadBytes(max);
			int count = 0;
			for (; count < max && data[count] != 0; count++) ;
			return encoding.GetString(data, 0, count);
		}
	}
}