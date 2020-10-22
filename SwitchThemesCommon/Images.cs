using ExtensionMethods;
using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace SwitchThemes.Common.Images
{
	public enum ImageFormat
	{
		Unknown, Dds, Jpg, Png
	}

	public struct ImageSize
	{
		readonly public uint Width, Height;

		public void Deconstruct(out uint w, out uint y) =>
			(w, y) = (Width, Height);

		public ImageSize(uint w, uint h) =>
			(Width, Height) = (w, h);
	}

	public interface IImageInfo 
	{
		ImageSize Size { get; }
		string Extension { get; }
		ImageFormat Format { get; }

		void AssertValidForBG();
		void AssertValidForApplet();
	}

	public struct JpgInfo : IImageInfo
	{
		public bool IsProgressive;
		public ImageSize Size { get; internal set; }

		public string Extension => "jpg";
		public ImageFormat Format => ImageFormat.Jpg;

		public JpgInfo(bool isProgressive, uint width, uint height)
		{
			IsProgressive = isProgressive;
			Size = new ImageSize(width, height);
		}

		public void Deconstruct(out uint w, out uint y, out bool p)
		{
			(w, y) = Size;
			p = IsProgressive;
		}

		public void AssertValidForBG() 
		{
			if (IsProgressive)
				throw new Exception("Jpg images are not supported when progressive encoding is enabled, check your image editor settings and disable it");
		}

		public void AssertValidForApplet() =>
			throw new Exception("Jpg images can't be used for applet icons");
	}

	public struct PngInfo : IImageInfo
	{
		public ImageSize Size { get; internal set; }
		
		public string Extension => "png";
		public ImageFormat Format => ImageFormat.Png;

		public void AssertValidForApplet() { }

		public void AssertValidForBG() =>
			throw new Exception("Png images can't be used as background images");
	}

	public class DDS : IImageInfo
	{
		public struct Header : IImageInfo
		{
			public string Encoding;
			public int MipmapCount;
			public ImageSize Size { get; internal set; }

			public string Extension => "dds";
			public ImageFormat Format => ImageFormat.Dds;

			public uint PixelDataLength;

			public void AssertValidForBG()
			{
				if (Encoding != "DXT1")
					throw new Exception("Only DXT1-encoded Dds files can be used as background images");
			}

			public void AssertValidForApplet()
			{
				if (Encoding != "DXT1" && Encoding != "DXT4" && Encoding != "DXT5" && Encoding != "DXT3")
					throw new Exception("Only DXT1/3/4/5 encodings are supported for dds applet icons.");
			}
		}

		internal struct EncoderInfo
		{
			public int blkHeight;
			public int blkWidth;
			public int bpp;
			public int formatCode;
		}

		internal static readonly Dictionary<string, EncoderInfo> EncoderTable = new Dictionary<string, EncoderInfo>() {
			{ "DXT1", new EncoderInfo() { blkHeight = 4, blkWidth = 4, bpp = 8 , formatCode = 0x1a01 } },
			{ "DXT3", new EncoderInfo() { blkHeight = 4, blkWidth = 4, bpp = 16 , formatCode = 0x1b01 } },
			{ "DXT4", new EncoderInfo() { blkHeight = 4, blkWidth = 4, bpp = 16 , formatCode = 0x1c01 } },
			{ "DXT5", new EncoderInfo() { blkHeight = 4, blkWidth = 4, bpp = 16 , formatCode = 0x1c01 } },
		};

		readonly public Header Info;
		readonly public byte[] Data;

		public int Height => (int)Info.Size.Height;
		public int Width => (int)Info.Size.Width;

		public DDS(byte[] data)
		{
			Info = Util.ParseDds(data);
			uint mipSize = 0; // not implemented
			Data = new byte[Info.PixelDataLength + mipSize];
			Array.Copy(data, 0x80, Data, 0, Info.PixelDataLength + mipSize);
		}

		public ImageSize Size => Info.Size;
		public string Extension => Info.Extension;
		public ImageFormat Format => Info.Format;
		public void AssertValidForBG() => Info.AssertValidForBG();
		public void AssertValidForApplet() => Info.AssertValidForApplet();
	}

	public static class Util
	{
		public static ImageFormat DetectFormat(byte[] data)
		{
			if (data.Matches("DDS "))
				return ImageFormat.Dds;
			else if (data.Matches(0, new byte[] { 0xFF, 0xD8, 0xFF }))
				return ImageFormat.Jpg;
			else if (data.Matches(1, "PNG"))
				return ImageFormat.Png;
			return ImageFormat.Unknown;
		}

		public static IImageInfo ParseImage(byte[] data)
		{
			switch (DetectFormat(data))
			{
				case ImageFormat.Dds:
					return ParseDds(data);
				case ImageFormat.Jpg:
					return ParseJpg(data);
				case ImageFormat.Png:
					return ParsePng(data);
				default:
					throw new Exception("Image format not supported");
			}
		}

		public static PngInfo ParsePng(byte[] data)
		{
			uint w, h;
			using (BinaryDataReader bin = new BinaryDataReader(new MemoryStream(data)))
			{
				bin.ByteOrder = ByteOrder.BigEndian;
				bin.BaseStream.Position = 0x10;
				w = bin.ReadUInt32();
				h = bin.ReadUInt32();
			}
			return new PngInfo { Size = new ImageSize(w,h) };
		}		

		public static JpgInfo ParseJpg(byte[] data)
		{
			uint w = 0, h = 0;
			bool Progressive = false;
			using (BinaryDataReader bin = new BinaryDataReader(new MemoryStream(data)))
			{
				bin.ByteOrder = ByteOrder.BigEndian;
				while (bin.BaseStream.Position < bin.BaseStream.Length)
				{
					byte marker = 0;
					while ((marker = bin.ReadByte()) != 0xFF) ;
					while ((marker = bin.ReadByte()) == 0xFF) ;

					if (marker == 0xC0)
					{
						bin.ReadByte();
						bin.ReadByte();
						bin.ReadByte();

						h = bin.ReadUInt16();
						w = bin.ReadUInt16();
					}
					if (marker == 0xC2)
					{
						Progressive = true;
					}
				}
			}
			return new JpgInfo(Progressive, w, h);
		}		

		public static DDS.Header ParseDds(byte[] data)
		{
			string FormatMagic = "" + (char)data[0x54] + (char)data[0x55] + (char)data[0x56] + (char)data[0x57];

			if (!DDS.EncoderTable.ContainsKey(FormatMagic))
				throw new Exception("Unsupported DDS format");

			var bpp = DDS.EncoderTable[FormatMagic].bpp;

			var width = BitConverter.ToUInt32(data, 0x10);
			var height = BitConverter.ToUInt32(data, 0xC);
			uint size = ((width + 3) >> 2) * ((height + 3) >> 2) * (uint)bpp;
			var numMips = 0; // Not implemented
			return new DDS.Header()
			{
				Size = new ImageSize(width, height),
				Encoding = FormatMagic,
				PixelDataLength = size,
				MipmapCount = numMips,
			};
		}
	}

	public static class Validation
	{
		private static void AssertBGSizeValid(ImageSize size)
		{
			if (size.Width != 1280 || size.Height != 720)
				throw new Exception("The background image must be 1280x720.");
		}

		private static void AssertBGSizeValid(this IImageInfo size) =>
			AssertBGSizeValid(size.Size);

		public static IImageInfo AssertValidForBG(byte[] data)
		{
			var img = Util.ParseImage(data);

			img.AssertValidForBG();
			img.AssertBGSizeValid();

			return img;
		}

		private static void AssertAppletSizeValid(TextureReplacement repl, ImageSize size)
		{
			if (size.Width != repl.W || size.Height != repl.H)
				throw new Exception($"The applet image size for {repl.NxThemeName} must be {repl.W}x{repl.H}");
		}

		private static void AssertAppletSizeValid(this IImageInfo size, TextureReplacement repl) =>
			AssertAppletSizeValid(repl, size.Size);

		public static IImageInfo AssertValidForApplet(TextureReplacement target, byte[] data) 
		{
			var img = Util.ParseImage(data);

			img.AssertValidForApplet();
			img.AssertAppletSizeValid(target);

			return img;
		}
	}
}
