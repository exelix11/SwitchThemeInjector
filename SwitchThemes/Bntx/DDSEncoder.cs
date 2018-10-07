using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SwitchThemes.Bntx
{
	//mostly based on https://github.com/aboood40091/BNTX-Editor
	public static class DDSEncoder  //Hardcoded valuses for DXT1 1280x720
	{
		public class DDSLoadResult
		{
			public int width;
			public int height;
			public int format_;
			public string fourcc;
			public int size;
			public int[] compSel;
			public int numMips;
			public byte[] data;
		}

		public static byte[] EncodeTex(DDSLoadResult img)
		{
			var numMips = 1;
			var alignment = 512;
			var blkWidth = 4;
			var blkHeight = 4;
			var bpp = 8;
			var blockHeight = Utils.getBlockHeight(Utils.DIV_ROUND_UP(img.height, blkHeight));
			var blockHeightLog2 = Utils.Log2(blockHeight);
			var linesPerBlockHeight = blockHeight * 8;

			var surfSize = 0;
			var blockHeightShift = 0;

			List<int> mipOfsets = new List<int>();
			List<byte> res = new List<byte>();

			for (int mipLevel = 0; mipLevel < numMips; mipLevel++)
			{
				var offSize = getCurrentMipOffset_Size(img.width, img.height, blkWidth, blkHeight, bpp, mipLevel);
				byte[] data = new byte[offSize.Item2];
				Array.Copy(img.data, offSize.Item1, data, 0, offSize.Item2);
				var width_ = Math.Max(1, img.width >> mipLevel);
				var height_ = Math.Max(1, img.height >> mipLevel);
				var width__ = Utils.DIV_ROUND_UP(width_, blkWidth);
				var height__ = Utils.DIV_ROUND_UP(height_, blkHeight);
				int dataAlignBytes = Utils.round_up(surfSize, alignment) - surfSize;
				surfSize += dataAlignBytes;
				mipOfsets.Add(surfSize);
				if (Utils.pow2_round_up(height__) < linesPerBlockHeight)
					blockHeightShift += 1;
				var pitch = Utils.round_up(width__ * bpp, 64);
				surfSize += pitch * Utils.round_up(height__, Math.Max(1, blockHeight >> blockHeightShift) * 8);

				if (dataAlignBytes != 0)
					res.AddRange(new byte[dataAlignBytes]);
				res.AddRange(Swizzle.swizzle(width_, height_, blkWidth, blkHeight, true, bpp, 0, Math.Max(0, blockHeightLog2 - blockHeightShift), data, true));
			}

			return res.ToArray();
		}

		static Tuple<int, int> getCurrentMipOffset_Size(int width, int height, int blkWidth, int blkHeight, int bpp, int currLevel)
		{
			var offset = 0;
			var width_ = 0;
			var height_ = 0;

			for (int mipLevel = 0; mipLevel < currLevel; mipLevel++)
			{
				width_ = Utils.DIV_ROUND_UP(Math.Max(1, width >> mipLevel), blkWidth);
				height_ = Utils.DIV_ROUND_UP(Math.Max(1, height >> mipLevel), blkHeight);
				offset += width_ * height_ * bpp;
			}

			width_ = Utils.DIV_ROUND_UP(Math.Max(1, width >> currLevel), blkWidth);
			height_ = Utils.DIV_ROUND_UP(Math.Max(1, height >> currLevel), blkHeight);
			var size = width_ * height_ * bpp;

			return new Tuple<int, int>(offset, size);
		}

		public static DDSLoadResult LoadDDS(byte[] inb)
		{
			if (!(inb[0x54] == 'D' && inb[0x55] == 'X' && inb[0x56] == 'T' && inb[0x57] == '1'))
				throw new Exception("Unsupported format : only DXT1 encoding is supported for DDS");

			var format_ = 0x1a06;
			var bpp = 8;
			var width = BitConverter.ToInt32(inb,0x10);
			var height = BitConverter.ToInt32(inb, 0xC);
			var size = ((width + 3) >> 2) * ((height + 3) >> 2) * bpp;
			var numMips = 0;
			var mipSize = 0;
			byte[] res = new byte[size + mipSize];
			Array.Copy(inb, 0x80, res, 0, size + mipSize);
			return new DDSLoadResult()
			{
				width = width,
				height = height,
				format_ = format_,
				fourcc = "DXT1",
				size = size,
				compSel = new int[] { 2, 3, 4, 5 },
				numMips = numMips,
				data = res
			};
		}

		static class Utils
		{
			public static int DIV_ROUND_UP(int n, int d) => (n + d - 1) / d;
			public static int round_up(int x, int y) => ((x - 1) | (y - 1)) + 1;

			public static int pow2_round_up(int x)
			{
				x -= 1;
				x |= x >> 1;
				x |= x >> 2;
				x |= x >> 4;
				x |= x >> 8;
				x |= x >> 16;
				return x + 1;
			}

			public static int Log2(int v)
			{
				int r = 0xFFFF - v >> 31 & 0x10;
				v >>= r;
				int shift = 0xFF - v >> 31 & 0x8;
				v >>= shift;
				r |= shift;
				shift = 0xF - v >> 31 & 0x4;
				v >>= shift;
				r |= shift;
				shift = 0x3 - v >> 31 & 0x2;
				v >>= shift;
				r |= shift;
				r |= (v >> 1);
				return r;
			}

			public static int getBlockHeight(int height)
			{
				var blockHeight = Utils.pow2_round_up(height / 8);

				if (blockHeight > 16)
					blockHeight = 16;

				return blockHeight;
			}
		}

		static class Swizzle
		{
			static int getAddrBlockLinear(int x, int y, int image_width, int bytes_per_pixel, int base_address, int blockHeight)
			{
				var image_width_in_gobs = Utils.DIV_ROUND_UP(image_width * bytes_per_pixel, 64);
				var GOB_address = (base_address
					   + (y / (8 * blockHeight)) * 512 * blockHeight * image_width_in_gobs
					   + (x * bytes_per_pixel / 64) * 512 * blockHeight
						+ (y % (8 * blockHeight) / 8) * 512);
				x *= bytes_per_pixel;
				return (GOB_address + ((x % 64) / 32) * 256 + ((y % 8) / 2) * 64 + ((x % 32) / 16) * 32 + (y % 2) * 16 + (x % 16));
			}

			public static byte[] swizzle(int width, int height, int blkWidth, int blkHeight, bool roundPitch, int bpp, int tileMode, int blockHeightLog2, byte[] data, bool toSwizzle)
			{
				var blockHeight = 1 << blockHeightLog2;
				width = Utils.DIV_ROUND_UP(width, blkWidth);
				height = Utils.DIV_ROUND_UP(height, blkHeight);

				var pitch = -1;
				var surfSize = -1;
				if (tileMode == 1)
				{
					pitch = width * bpp;
					if (roundPitch)
						pitch = Utils.round_up(pitch, 32);
					surfSize = pitch * height;
				}
				else
				{
					pitch = Utils.round_up(width * bpp, 64);
					surfSize = pitch * Utils.round_up(height, blockHeight * 8);
				}
				byte[] res = new byte[surfSize];
				for (int y = 0; y < height; y++)
					for (int x = 0; x < width; x++)
					{
						var pos = -1;
						if (tileMode == 1)
							pos = y * pitch + x * bpp;
						else
							pos = getAddrBlockLinear(x, y, width, bpp, 0, blockHeight);
						var pos_ = (y * width + x) * bpp;
						if (pos + bpp <= surfSize)
						{
							if (toSwizzle)
								Array.Copy(data, pos_, res, pos, bpp);
							else
								Array.Copy(data, pos, res, pos_, bpp);
						}
					}
				return res;
			}
		}
	}
}
